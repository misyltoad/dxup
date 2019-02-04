#include "d3d9_device.h"
#include "d3d9_interface.h"
#include "d3d9_swapchain.h"
#include "d3d9_surface.h"
#include "d3d9_texture.h"
#include "d3d9_buffer.h"
#include "d3d9_shaders.h"
#include "../dx9asm/dx9asm_translator.h"
#include "../util/config.h"
#include "../util/d3dcompiler_helpers.h"
#include "d3d9_resource.h"
#include "d3d9_vertexdeclaration.h"
#include "d3d9_state_cache.h"
#include "d3d9_d3d11_resource.h"
#include "../util/hash.h"
#include "d3d9_query.h"
#include "d3d9_state.h"
#include "d3d9_renderer.h"
#include <d3d11_4.h>
#include <float.h>

namespace dxup {

  Direct3DDevice9Ex::Direct3DDevice9Ex(
    UINT adapterNum,
    IDXGIAdapter1* adapter,
    HWND window,
    ID3D11Device1* device,
    ID3D11DeviceContext1* context,
    Direct3D9Ex* parent,
    D3DDEVTYPE deviceType,
    DWORD behaviourFlags,
    uint8_t flags
  )
    : m_adapterNum{ adapterNum }
    , m_adapter{ adapter }
    , m_window{ window }
    , m_device{ device }
    , m_context{ context }
    , m_parent{ parent }
    , m_behaviourFlags{ behaviourFlags }
    , m_flags{ flags }
    , m_deviceType{ deviceType }
    , m_state{ new D3D9State(this, 0) }
    , m_stateBlock{ nullptr } {
    m_renderer = new D3D9ImmediateRenderer{ device, context, m_state };
    InitializeCriticalSection(&m_criticalSection);

    if (!(behaviourFlags & D3DCREATE_FPU_PRESERVE))
      setupFPUFlags();
  }

  HRESULT Direct3DDevice9Ex::CreateD3D11Device(UINT adapter, Direct3D9Ex* parent, ID3D11Device1** device, ID3D11DeviceContext1** context, IDXGIDevice1** dxgiDevice, IDXGIAdapter1** dxgiAdapter) {
    if (adapter >= parent->getAdapterList().size())
      return log::d3derr(D3DERR_INVALIDCALL, "CreateD3D11Device: adapter out of bounds (adapter = %d, range: 0-%d).", adapter, parent->getAdapterList().size());

    *dxgiAdapter = ref(parent->getAdapterList()[adapter].ptr());

    UINT Flags = D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT | D3D11_CREATE_DEVICE_BGRA_SUPPORT; // Why isn't this a default?! ~ Josh

    if (config::getBool(config::Debug))
      Flags |= D3D11_CREATE_DEVICE_DEBUG;

    std::array<D3D_FEATURE_LEVEL, 4> featureLevels =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_1;

    Com<ID3D11Device> initialDevice;
    Com<ID3D11DeviceContext> initialContext;

    HRESULT result = D3D11CreateDevice(
      *dxgiAdapter,
      D3D_DRIVER_TYPE_UNKNOWN,
      nullptr,
      Flags,
      featureLevels.data(),
      featureLevels.size(),
      D3D11_SDK_VERSION,
      &initialDevice,
      &level,
      &initialContext
    );

    if (FAILED(result))
      return log::d3derr(D3DERR_DEVICELOST, "Device creation: failed to create D3D11 device.");

    result = initialDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)device);
    HRESULT contextResult = initialContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)context);

    if (FAILED(result) || FAILED(contextResult))
      return log::d3derr(D3DERR_DEVICELOST, "Device creation: unable to upgrade D3D11 device to D3D11_1 device.");

    result = initialDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)dxgiDevice);

    if (FAILED(result))
      return log::d3derr(D3DERR_DEVICELOST, "Device creation: couldn't obtain IDXGIDevice1 from D3D11 device.");

    return D3D_OK;
  }

  HRESULT Direct3DDevice9Ex::Create(
    UINT adapter,
    HWND window,
    Direct3D9Ex* parent,
    D3DPRESENT_PARAMETERS* presentParameters,
    D3DDEVTYPE deviceType,
    bool isEx,
    DWORD behaviourFlags,
    IDirect3DDevice9Ex** outDevice
    ) {
    InitReturnPtr(outDevice);

    if (outDevice == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "Device creation: outDevice as null.");

    if (presentParameters == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "Device creation: presentParameters was nullptr.");

    Com<ID3D11Device1> device;
    Com<ID3D11DeviceContext1> context;
    Com<IDXGIAdapter1> dxgiAdapter;
    Com<IDXGIDevice1> dxgiDevice;

    HRESULT result = CreateD3D11Device(adapter, parent, &device, &context, &dxgiDevice, &dxgiAdapter);

    if (FAILED(result))
      return result;

    SetupD3D11Debug(device.ptr());

    if (behaviourFlags & D3DCREATE_MULTITHREADED) {
      Com<ID3D11Multithread> d3d11Multithread;
      device->QueryInterface(__uuidof(ID3D11Multithread), reinterpret_cast<void**>(&d3d11Multithread));

      if (d3d11Multithread != nullptr)
        d3d11Multithread->SetMultithreadProtected(true);
      else
        log::warn("Failed to respect D3D9 multithread parameter.");
    }

    if (FAILED(result))
      return result;

    uint8_t flags = 0;

    if (isEx)
      flags |= DeviceFlag_Ex;

    RECT rect;
    GetWindowRect(window, &rect);

    if (!presentParameters->BackBufferWidth)
      presentParameters->BackBufferWidth = rect.right;

    if (!presentParameters->BackBufferHeight)
      presentParameters->BackBufferHeight = rect.bottom;

    if (!presentParameters->BackBufferCount)
      presentParameters->BackBufferCount = 1;

    if (presentParameters->BackBufferFormat == D3DFMT_UNKNOWN)
      presentParameters->BackBufferFormat = D3DFMT_A8B8G8R8;

    Direct3DDevice9Ex* d3d9Device = ref(new Direct3DDevice9Ex(
      adapter,
      dxgiAdapter.ptr(),
      window,
      device.ptr(),
      context.ptr(),
      parent,
      deviceType,
      behaviourFlags,
      flags)); // We must ref before reset or we get deleted due to internal ref changes.

    result = d3d9Device->Reset(presentParameters);

    if (FAILED(result)) {
      delete d3d9Device;
      return log::d3derr(D3DERR_INVALIDCALL, "Device creation: initial reset failed."); 
    }

    *outDevice = d3d9Device;

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {
    CriticalSection cs(this);

    if (pPresentationParameters == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "Reset: pPresentationParameters was nullptr.");

    // Unbind current state...
    SetDepthStencilSurface(nullptr);

    for (uint32_t i = 0; i < 4; i++)
      SetRenderTarget(0, nullptr);

    // Setup new state...

    HRESULT result = D3D_OK;

    // Defaults from SwiftShader.
    SetRenderState(D3DRS_ZENABLE, pPresentationParameters->EnableAutoDepthStencil != FALSE ? D3DZB_TRUE : D3DZB_FALSE);
    SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    SetRenderState(D3DRS_LASTPIXEL, TRUE);
    SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
    SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    SetRenderState(D3DRS_ALPHAREF, 0);
    SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
    SetRenderState(D3DRS_DITHERENABLE, FALSE);
    SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    SetRenderState(D3DRS_FOGENABLE, FALSE);
    SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    //	SetRenderState(D3DRS_ZVISIBLE, 0);
    SetRenderState(D3DRS_FOGCOLOR, 0);
    SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
    SetRenderState(D3DRS_FOGSTART, reinterpret::floatToDword(0.0f));
    SetRenderState(D3DRS_FOGEND, reinterpret::floatToDword(1.0f));
    SetRenderState(D3DRS_FOGDENSITY, reinterpret::floatToDword(1.0f));
    SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
    SetRenderState(D3DRS_STENCILENABLE, FALSE);
    SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
    SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
    SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
    SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
    SetRenderState(D3DRS_STENCILREF, 0);
    SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
    SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
    SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
    SetRenderState(D3DRS_WRAP0, 0);
    SetRenderState(D3DRS_WRAP1, 0);
    SetRenderState(D3DRS_WRAP2, 0);
    SetRenderState(D3DRS_WRAP3, 0);
    SetRenderState(D3DRS_WRAP4, 0);
    SetRenderState(D3DRS_WRAP5, 0);
    SetRenderState(D3DRS_WRAP6, 0);
    SetRenderState(D3DRS_WRAP7, 0);
    SetRenderState(D3DRS_CLIPPING, TRUE);
    SetRenderState(D3DRS_LIGHTING, TRUE);
    SetRenderState(D3DRS_AMBIENT, 0);
    SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
    SetRenderState(D3DRS_COLORVERTEX, TRUE);
    SetRenderState(D3DRS_LOCALVIEWER, TRUE);
    SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
    SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);
    SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
    SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
    SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
    SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
    SetRenderState(D3DRS_POINTSIZE, reinterpret::floatToDword(1.0f));
    SetRenderState(D3DRS_POINTSIZE_MIN, reinterpret::floatToDword(1.0f));
    SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
    SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
    SetRenderState(D3DRS_POINTSCALE_A, reinterpret::floatToDword(1.0f));
    SetRenderState(D3DRS_POINTSCALE_B, reinterpret::floatToDword(0.0f));
    SetRenderState(D3DRS_POINTSCALE_C, reinterpret::floatToDword(0.0f));
    SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
    SetRenderState(D3DRS_MULTISAMPLEMASK, 0xFFFFFFFF);
    SetRenderState(D3DRS_PATCHEDGESTYLE, D3DPATCHEDGE_DISCRETE);
    SetRenderState(D3DRS_DEBUGMONITORTOKEN, D3DDMT_ENABLE);
    SetRenderState(D3DRS_POINTSIZE_MAX, reinterpret::floatToDword(64.0f));
    SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
    SetRenderState(D3DRS_COLORWRITEENABLE, 0x0000000F);
    SetRenderState(D3DRS_TWEENFACTOR, reinterpret::floatToDword(0.0f));
    SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    SetRenderState(D3DRS_POSITIONDEGREE, D3DDEGREE_CUBIC);
    SetRenderState(D3DRS_NORMALDEGREE, D3DDEGREE_LINEAR);
    SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, reinterpret::floatToDword(0.0f));
    SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);
    SetRenderState(D3DRS_MINTESSELLATIONLEVEL, reinterpret::floatToDword(1.0f));
    SetRenderState(D3DRS_MAXTESSELLATIONLEVEL, reinterpret::floatToDword(1.0f));
    SetRenderState(D3DRS_ADAPTIVETESS_X, reinterpret::floatToDword(0.0f));
    SetRenderState(D3DRS_ADAPTIVETESS_Y, reinterpret::floatToDword(0.0f));
    SetRenderState(D3DRS_ADAPTIVETESS_Z, reinterpret::floatToDword(1.0f));
    SetRenderState(D3DRS_ADAPTIVETESS_W, reinterpret::floatToDword(0.0f));
    SetRenderState(D3DRS_ENABLEADAPTIVETESSELLATION, FALSE);
    SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE);
    SetRenderState(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP);
    SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
    SetRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_KEEP);
    SetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_ALWAYS);
    SetRenderState(D3DRS_COLORWRITEENABLE1, 0x0000000F);
    SetRenderState(D3DRS_COLORWRITEENABLE2, 0x0000000F);
    SetRenderState(D3DRS_COLORWRITEENABLE3, 0x0000000F);
    SetRenderState(D3DRS_BLENDFACTOR, 0xFFFFFFFF);
    SetRenderState(D3DRS_SRGBWRITEENABLE, 0);
    SetRenderState(D3DRS_DEPTHBIAS, reinterpret::floatToDword(0.0f));
    SetRenderState(D3DRS_WRAP8, 0);
    SetRenderState(D3DRS_WRAP9, 0);
    SetRenderState(D3DRS_WRAP10, 0);
    SetRenderState(D3DRS_WRAP11, 0);
    SetRenderState(D3DRS_WRAP12, 0);
    SetRenderState(D3DRS_WRAP13, 0);
    SetRenderState(D3DRS_WRAP14, 0);
    SetRenderState(D3DRS_WRAP15, 0);
    SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
    SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
    SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
    SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
    for (uint32_t i = 0; i < 8; i++)
    {
      SetTextureStageState(i, D3DTSS_COLOROP, i == 0 ? D3DTOP_MODULATE : D3DTOP_DISABLE);
      SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT);
      SetTextureStageState(i, D3DTSS_ALPHAOP, i == 0 ? D3DTOP_SELECTARG1 : D3DTOP_DISABLE);
      SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
      SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
      SetTextureStageState(i, D3DTSS_BUMPENVMAT00, reinterpret::floatToDword(0.0f));
      SetTextureStageState(i, D3DTSS_BUMPENVMAT01, reinterpret::floatToDword(0.0f));
      SetTextureStageState(i, D3DTSS_BUMPENVMAT10, reinterpret::floatToDword(0.0f));
      SetTextureStageState(i, D3DTSS_BUMPENVMAT11, reinterpret::floatToDword(0.0f));
      SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, i);
      SetTextureStageState(i, D3DTSS_BUMPENVLSCALE, reinterpret::floatToDword(0.0f));
      SetTextureStageState(i, D3DTSS_BUMPENVLOFFSET, reinterpret::floatToDword(0.0f));
      SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
      SetTextureStageState(i, D3DTSS_COLORARG0, D3DTA_CURRENT);
      SetTextureStageState(i, D3DTSS_ALPHAARG0, D3DTA_CURRENT);
      SetTextureStageState(i, D3DTSS_RESULTARG, D3DTA_CURRENT);
      SetTextureStageState(i, D3DTSS_CONSTANT, 0x00000000);
    }

    forEachSampler([&](uint32_t i)
    {
      SetTexture(i, 0);
      SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
      SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
      SetSamplerState(i, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
      SetSamplerState(i, D3DSAMP_BORDERCOLOR, 0x00000000);
      SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
      SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_POINT);
      SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
      SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, 0);
      SetSamplerState(i, D3DSAMP_MAXMIPLEVEL, 0);
      SetSamplerState(i, D3DSAMP_MAXANISOTROPY, 1);
      SetSamplerState(i, D3DSAMP_SRGBTEXTURE, 0);
      SetSamplerState(i, D3DSAMP_ELEMENTINDEX, 0);
      SetSamplerState(i, D3DSAMP_DMAPOFFSET, 0);
    });

    for (uint32_t i = 0; i < 6; i++) {
      float plane[4] = { 0, 0, 0, 0 };
      SetClipPlane(i, plane);
    }

    m_renderer->undirtyContext(); // This should free up the swapchain SRVs on the d3d11 context.

    if (GetInternalSwapchain(0) == nullptr) {
      result = CreateAdditionalSwapChain(pPresentationParameters, (IDirect3DSwapChain9**)&m_swapchains[0]);

      if (FAILED(result))
        return log::d3derr(D3DERR_INVALIDCALL, "Reset: implicit swapchain could not be created.");
    }
    else {
      result = GetInternalSwapchain(0)->Reset(pPresentationParameters);

      if (FAILED(result))
        return log::d3derr(D3DERR_INVALIDCALL, "Reset: implicit swapchain failed to be reset.");
    }

    Com<IDirect3DSurface9> backbuffer;
    result = m_swapchains[0]->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);

    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "Reset: swapchain's backbuffer could not be retrieved.");

    SetRenderTarget(0, backbuffer.ptr());

    if (pPresentationParameters->EnableAutoDepthStencil) {
      Com<IDirect3DSurface9> autoDepthStencil;
      CreateDepthStencilSurface(pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight, pPresentationParameters->AutoDepthStencilFormat, pPresentationParameters->MultiSampleType, pPresentationParameters->MultiSampleQuality, false, &autoDepthStencil, nullptr);
      SetDepthStencilSurface(autoDepthStencil.ptr());
    }

    if (config::getBool(config::InitialHideCursor))
      ShowCursor(false);

    return D3D_OK;
  }

  void Direct3DDevice9Ex::SetupD3D11Debug(ID3D11Device* device) {
    if (config::getBool(config::Debug)) {

      Com<ID3D11Debug> d3dDebug;
      if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug))) {

        Com<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue))) {

          d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
          d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

          std::array<D3D11_MESSAGE_ID, 2> messagesToHide = {
            D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
            D3D11_MESSAGE_ID_OMSETRENDERTARGETS_INVALIDVIEW, // This is a validation error for some reason but it does not actually matter and is fully supported.
          };

          D3D11_INFO_QUEUE_FILTER filter;
          memset(&filter, 0, sizeof(filter));
          filter.DenyList.NumIDs = messagesToHide.size();
          filter.DenyList.pIDList = &messagesToHide[0];
          d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
      }

    }
  }

  Direct3DDevice9Ex::~Direct3DDevice9Ex() {
    DeleteCriticalSection(&m_criticalSection);
    delete m_state;
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::QueryInterface(REFIID riid, LPVOID* ppv) {
    InitReturnPtr(ppv);

    if (ppv == nullptr)
      return E_POINTER;

    if (riid == __uuidof(IDirect3DDevice9Ex) || riid == __uuidof(IDirect3DDevice9) || riid == __uuidof(IUnknown))
      *ppv = ref(this);

    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::TestCooperativeLevel() {
    CriticalSection cs(this);

    if (m_flags & DeviceFlag_Ex)
      return D3D_OK;

    Direct3DSwapChain9Ex* swapchain = GetInternalSwapchain(0);

    if (swapchain == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "TestCooperativeLevel: swapchain0 was nullptr.");

    return swapchain->TestSwapchain(nullptr, 0);
  }

  UINT    STDMETHODCALLTYPE Direct3DDevice9Ex::GetAvailableTextureMem() {
    CriticalSection cs(this);

    DXGI_ADAPTER_DESC adapterDesc; 
    m_adapter->GetDesc(&adapterDesc);
    return UINT(adapterDesc.DedicatedVideoMemory / 1024ul);
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::EvictManagedResources() {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::EvictManagedResources");
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetDirect3D(IDirect3D9** ppD3D9) {
    CriticalSection cs(this);

    if (!ppD3D9)
      return log::d3derr(D3DERR_INVALIDCALL, "GetDirect3D: ppD3D9 was nullptr.");
    *ppD3D9 = ref(m_parent);

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetDeviceCaps(D3DCAPS9* pCaps) {
    CriticalSection cs(this);

    return m_parent->GetDeviceCaps(0, D3DDEVTYPE_HAL, pCaps);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {
    CriticalSection cs(this);

    if (GetInternalSwapchain(iSwapChain) == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetDisplayMode: requested swapchain doesn't exist");

    return GetInternalSwapchain(iSwapChain)->GetDisplayMode(pMode);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) {
    CriticalSection cs(this);

    if (!pParameters)
      return log::d3derr(D3DERR_INVALIDCALL, "GetCreationParameters: pParameters was nullptr.");

    pParameters->AdapterOrdinal = m_adapterNum;
    pParameters->BehaviorFlags = m_behaviourFlags;
    pParameters->hFocusWindow = m_window;
    pParameters->DeviceType = m_deviceType;

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::SetCursorProperties");
    return D3D_OK;
  }
  void    STDMETHODCALLTYPE Direct3DDevice9Ex::SetCursorPosition(int X, int Y, DWORD Flags) {
    CriticalSection cs(this);

    if (Flags & D3DCURSOR_IMMEDIATE_UPDATE) {
      ::SetCursorPos(X, Y);
      m_pendingCursorUpdate = { 0 };
      return;
    }

    m_pendingCursorUpdate.update = true;
    m_pendingCursorUpdate.x = X;
    m_pendingCursorUpdate.y = Y;
  }
  BOOL    STDMETHODCALLTYPE Direct3DDevice9Ex::ShowCursor(BOOL bShow) {
    CriticalSection cs(this);

    // Ok so if they call FALSE here it means they want to use the regular windows cursor.
    // if they call TRUE here it means they want to use some weird bitmap cursor that I currently dont care about.
    // Therefore we always want to show the regular cursor no matter what!
    ::ShowCursor(true);

    return TRUE;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** ppSwapChain) {
    CriticalSection cs(this);
    InitReturnPtr(ppSwapChain);

    bool full = true;
    for (size_t i = 0; i < m_swapchains.size(); i++) {
      if (m_swapchains[i] == nullptr)
        full = false;
    }

    if (full)
      return log::d3derr(D3DERR_INVALIDCALL, "CreateAdditionalSwapChain: no free swapchain slots.");

    Direct3DSwapChain9Ex** swapchain = reinterpret_cast<Direct3DSwapChain9Ex**>(ppSwapChain);
    HRESULT result = Direct3DSwapChain9Ex::Create(this, pPresentationParameters, swapchain);
    if (FAILED(result))
      return result;

    for (size_t i = 0; i < m_swapchains.size(); i++) {
      if (m_swapchains[i] == nullptr) {
        m_swapchains[i] = *swapchain;
        break;
      }
    }

    return D3D_OK;
  }

  Direct3DSwapChain9Ex* Direct3DDevice9Ex::GetInternalSwapchain(UINT i) {
    CriticalSection cs(this);

    if (i >= m_swapchains.size())
      return nullptr;

    return reinterpret_cast<Direct3DSwapChain9Ex*>(m_swapchains[i].ptr());
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {
    CriticalSection cs(this);

    InitReturnPtr(pSwapChain);
    if (pSwapChain == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetSwapChain: pSwapChain was nullptr.");

    if (GetInternalSwapchain(iSwapChain) == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetSwapChain: invalid swapchain requested (%d).", iSwapChain);

    *pSwapChain = ref(GetInternalSwapchain(iSwapChain));

    return D3D_OK;
  }
  UINT    STDMETHODCALLTYPE Direct3DDevice9Ex::GetNumberOfSwapChains() {
    CriticalSection cs(this);

    UINT swapchainCount = 0;

    for (size_t i = 0; i < m_swapchains.size(); i++) {
      if (m_swapchains[i] != nullptr)
        swapchainCount++;
    }
    return swapchainCount;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
    CriticalSection cs(this);

    m_renderer->endFrame();
    return PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, 0);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
    CriticalSection cs(this);

    Direct3DSwapChain9Ex* swapchain = GetInternalSwapchain(iSwapChain);

    if (swapchain == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetBackBuffer: invalid swapchain requested (%d).", iSwapChain);

    return swapchain->GetBackBuffer(iBackBuffer, Type, ppBackBuffer);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {
    CriticalSection cs(this);

    Direct3DSwapChain9Ex* swapchain = GetInternalSwapchain(iSwapChain);

    if (swapchain == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetRasterStatus: invalid swapchain requested (%d).", iSwapChain);

    return swapchain->GetRasterStatus(pRasterStatus);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetDialogBoxMode(BOOL bEnableDialogs) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::SetDialogBoxMode");
    return D3D_OK;
  }
  void    STDMETHODCALLTYPE Direct3DDevice9Ex::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::SetGammaRamp");
  }
  void    STDMETHODCALLTYPE Direct3DDevice9Ex::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::GetGammaRamp");
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
    CriticalSection cs(this);

    return Direct3DTexture9::Create(this, Width, Height, Levels, Usage, Format, Pool, (Direct3DTexture9**)ppTexture);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {
    CriticalSection cs(this);

    return Direct3DVolumeTexture9::Create(this, Width, Height, Depth, Levels, Usage, Format, Pool, (Direct3DVolumeTexture9**)ppVolumeTexture);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {
    CriticalSection cs(this);

    return Direct3DCubeTexture9::Create(this, EdgeLength, Levels, Usage, Format, Pool, (Direct3DCubeTexture9**)ppCubeTexture);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {
    CriticalSection cs(this);

    InitReturnPtr(ppVertexBuffer);
    InitReturnPtr(pSharedHandle);

    if (!ppVertexBuffer)
      return log::d3derr(D3DERR_INVALIDCALL, "CreateVertexBuffer: ppVertexBuffer was nullptr.");

    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = Length;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = convert::cpuFlags(Pool, Usage, D3DRTYPE_VERTEXBUFFER);
    desc.Usage = convert::usage(Pool, Usage, D3DRTYPE_VERTEXBUFFER);
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    Com<ID3D11Buffer> buffer;
    HRESULT result = m_device->CreateBuffer(&desc, nullptr, &buffer);
    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "CreateVertexBuffer: failed to create D3D11 buffer.");

    DXUPResource* resource = DXUPResource::Create(this, buffer.ptr(), Usage, D3DFMT_VERTEXDATA);
    if (resource == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "CreateVertexBuffer: failed to create DXUP resource.");

    D3D9ResourceDesc d3d9Desc;
    d3d9Desc.Pool = Pool;
    d3d9Desc.FVF = FVF;
    d3d9Desc.Usage = Usage;

    *ppVertexBuffer = ref(new Direct3DVertexBuffer9(this, resource, d3d9Desc));

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {
    CriticalSection cs(this);

    InitReturnPtr(ppIndexBuffer);
    InitReturnPtr(pSharedHandle);

    if (!ppIndexBuffer)
      return log::d3derr(D3DERR_INVALIDCALL, "CreateIndexBuffer: ppIndexBuffer was nullptr.");

    Usage &= ~D3DUSAGE_WRITEONLY; // We don't care about this for index buffers as we need staging on them for tri fan fixup.

    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = Length;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags = convert::cpuFlags(Pool, Usage, D3DRTYPE_INDEXBUFFER);
    desc.Usage = convert::usage(Pool, Usage, D3DRTYPE_INDEXBUFFER);
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    Com<ID3D11Buffer> buffer;
    HRESULT result = m_device->CreateBuffer(&desc, nullptr, &buffer);
    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "CreateIndexBuffer: failed to create D3D11 buffer.");

    DXUPResource* resource = DXUPResource::Create(this, buffer.ptr(), Usage, Format);
    if (resource == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "CreateIndexBuffer: failed to create DXUP resource.");

    D3D9ResourceDesc d3d9Desc;
    d3d9Desc.Pool = Pool;
    d3d9Desc.Format = Format;
    d3d9Desc.Usage = Usage;

    *ppIndexBuffer = ref(new Direct3DIndexBuffer9(this, resource, d3d9Desc));

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
    CriticalSection cs(this);

    return Direct3DSurface9::Create(this, Width, Height, D3DUSAGE_RENDERTARGET, Format, MultiSample, false, (Direct3DSurface9 **)ppSurface);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
    CriticalSection cs(this);

    return Direct3DSurface9::Create(this, Width, Height, D3DUSAGE_DEPTHSTENCIL, Format, MultiSample, Discard, (Direct3DSurface9**)ppSurface);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {
    CriticalSection cs(this);

    Direct3DSurface9* src = reinterpret_cast<Direct3DSurface9*>(pSourceSurface);
    Direct3DSurface9* dst = reinterpret_cast<Direct3DSurface9*>(pDestinationSurface);

    if (src == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "UpdateSurface: src was nullptr.");

    if (dst == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "UpdateSurface: dst was nullptr.");

    D3DSURFACE_DESC srcDesc;
    D3DSURFACE_DESC dstDesc;
    pSourceSurface->GetDesc(&srcDesc);
    pDestinationSurface->GetDesc(&dstDesc);

    D3D11_BOX srcBox;
    srcBox.front = 0;
    srcBox.back = 1;

    UINT dstX = 0;
    UINT dstY = 0;

    if (pSourceRect != nullptr) {
      srcBox.left = pSourceRect->left;
      srcBox.top = pSourceRect->top;
      srcBox.right = pSourceRect->right;
      srcBox.bottom = pSourceRect->bottom;

    }
    else {
      srcBox.left = 0;
      srcBox.top = 0;
      srcBox.right = srcDesc.Width;
      srcBox.bottom = srcDesc.Height;
    }

    if (pDestPoint != nullptr) {
      dstX = pDestPoint->x;
      dstY = pDestPoint->y;
    }

    if (!src->isBoxValid(&srcBox))
      return log::d3derr(D3DERR_INVALIDCALL, "UpdateSurface: box was invalid.");

    if (srcDesc.MultiSampleType != D3DMULTISAMPLE_NONE)
      return log::d3derr(D3DERR_INVALIDCALL, "UpdateSurface: src has multisampling.");

    if (dstDesc.MultiSampleType != D3DMULTISAMPLE_NONE)
      return log::d3derr(D3DERR_INVALIDCALL, "UpdateSurface: dst has multisampling.");

    if (srcDesc.Format != dstDesc.Format)
      return log::d3derr(D3DERR_INVALIDCALL, "UpdateSurface: src format is not the same as dst format.");

    // TODO: do we need to do staging here too? Look into this.
    m_context->CopySubresourceRegion(dst->GetDXUPResource()->GetResource(), dst->GetSubresource(), dstX, dstY, 0, src->GetDXUPResource()->GetResource(), src->GetSubresource(), &srcBox);
    dst->GetDXUPResource()->MarkDirty(dst->GetSlice(), dst->GetMip());
    
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) {
    CriticalSection cs(this);

    if (pSourceTexture == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "UpdateTexture: pSourceTexture was nullptr");
      
    if (pDestinationTexture == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "UpdateTexture: pDestinationTexture was nullptr");

    if (pSourceTexture->GetType() != pDestinationTexture->GetType())
      return log::d3derr(D3DERR_INVALIDCALL, "UpdateTexture: resource types don't match.");

    switch (pSourceTexture->GetType()) {

    case D3DRTYPE_TEXTURE: {

      IDirect3DTexture9* srcTex = reinterpret_cast<Direct3DTexture9*>(pSourceTexture);
      IDirect3DTexture9* dstTex = reinterpret_cast<Direct3DTexture9*>(pDestinationTexture);

      // TODO! Avoid COM here.
      Com<IDirect3DSurface9> srcSurface;
      Com<IDirect3DSurface9> dstSurface;

      srcTex->GetSurfaceLevel(0, &srcSurface);
      dstTex->GetSurfaceLevel(0, &dstSurface);

      StretchRect(srcSurface.ptr(), NULL, dstSurface.ptr(), NULL, D3DTEXF_NONE);
      dstTex->GenerateMipSubLevels();

      return D3D_OK;
    }

    default: return log::d3derr(D3DERR_INVALIDCALL, "UpdateTexture: unsupported resource type (%d).", pSourceTexture->GetType());
    }
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {
    CriticalSection cs(this);

    return UpdateSurface(pRenderTarget, nullptr, pDestSurface, nullptr);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
    CriticalSection cs(this);

    Direct3DSwapChain9Ex* swapchain = GetInternalSwapchain(iSwapChain);

    if (swapchain == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetFrontBufferData: invalid swapchain requested (%d).", iSwapChain);

    swapchain->GetFrontBufferData(pDestSurface);  
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {
    CriticalSection cs(this);

    Direct3DSurface9* src = reinterpret_cast<Direct3DSurface9*>(pSourceSurface);
    Direct3DSurface9* dst = reinterpret_cast<Direct3DSurface9*>(pDestSurface);

    if (pSourceRect != nullptr && pDestRect != nullptr) {
      UINT x = pDestRect->left;
      UINT y = pDestRect->top;

      // This doesn't properly handle 'stretching'.
      D3D11_BOX box;
      box.left = pSourceRect->left;
      box.right = pSourceRect->right;
      box.top = pSourceRect->top;
      box.bottom = pSourceRect->bottom;
      box.front = 0;
      box.back = 1;
      m_context->CopySubresourceRegion(dst->GetDXUPResource()->GetResource(), dst->GetSubresource(), x, y, 0, src->GetDXUPResource()->GetResource(), src->GetSubresource(), &box);
      dst->GetDXUPResource()->MarkDirty(dst->GetSlice(), dst->GetMip());

      return D3D_OK;
    }

    m_context->CopySubresourceRegion(dst->GetDXUPResource()->GetResource(), dst->GetSubresource(), 0, 0, 0, src->GetDXUPResource()->GetResource(), src->GetSubresource(), nullptr);
    dst->GetDXUPResource()->MarkDirty(dst->GetSlice(), dst->GetMip());

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {
    CriticalSection cs(this);

    if (pSurface == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "ColorFill: pSurface was nullptr.");

    Direct3DSurface9* src = reinterpret_cast<Direct3DSurface9*>(pSurface);

    if (src->GetD3D11RenderTarget(false) == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "ColorFill: pSurface had no rtv, is it sysmem or something?");

    float d3d11Color[4];
    convert::color(color, d3d11Color);

    m_context->ClearView(src->GetD3D11RenderTarget(false), d3d11Color, pRect, pRect == nullptr ? 0 : 1);

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
    CriticalSection cs(this);

    if (Pool == D3DPOOL_MANAGED)
      return log::d3derr(D3DERR_INVALIDCALL, "CreateOffscreenPlainSurface: attempted to make offscreen surface w/ managed pool.");

    return CreateRenderTarget(Width, Height, Format, D3DMULTISAMPLE_NONE, 0, Pool, ppSurface, pSharedHandle);
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
    CriticalSection cs(this);
    return GetEditState()->SetRenderTarget(RenderTargetIndex, pRenderTarget);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
    CriticalSection cs(this);
    return m_state->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {
    CriticalSection cs(this);
    m_renderer->handleDepthStencilDiscard(); // TODO! Does this only get done in d3d9 if it gets set.
    return GetEditState()->SetDepthStencilSurface(pNewZStencil);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
    CriticalSection cs(this);
    return m_state->GetDepthStencilSurface(ppZStencilSurface);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::BeginScene() {
    CriticalSection cs(this);
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::EndScene() {
    CriticalSection cs(this);
    m_context->Flush();
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
    CriticalSection cs(this);
    return m_renderer->Clear(Count, pRects, Flags, Color, Z, Stencil);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::SetTransform");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::GetTransform");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::MultiplyTransform(D3DTRANSFORMSTATETYPE TransformState, CONST D3DMATRIX* pMatrix) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::MultiplyTransform");
    return D3D_OK;
  }
  // TODO! Put viewport in state.
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetViewport(CONST D3DVIEWPORT9* pViewport) {
    CriticalSection cs(this);
    return GetEditState()->SetViewport(pViewport);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetViewport(D3DVIEWPORT9* pViewport) {
    CriticalSection cs(this);
    return m_state->GetViewport(pViewport);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetMaterial(CONST D3DMATERIAL9* pMaterial) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::SetMaterial");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetMaterial(D3DMATERIAL9* pMaterial) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::GetMaterial");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetLight(DWORD Index, CONST D3DLIGHT9* pLight) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::SetLight");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetLight(DWORD Index, D3DLIGHT9* pLight) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::GetLight");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::LightEnable(DWORD Index, BOOL Enable) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::LightEnable");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetLightEnable(DWORD Index, BOOL* pEnable) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::GetLightEnable");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetClipPlane(DWORD Index, CONST float* pPlane) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::SetClipPlane");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetClipPlane(DWORD Index, float* pPlane) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::GetClipPlane");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
    CriticalSection cs(this);
    return GetEditState()->SetRenderState(State, Value);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {
    CriticalSection cs(this);
    return m_state->GetRenderState(State, pValue);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {
    CriticalSection cs(this);
    InitReturnPtr(ppSB);
    if (ppSB == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "CreateStateBlock: ppSB was nullptr");;

    *ppSB = ref(new Direct3DStateBlock9(this, Type));
    
    return D3D_OK;
  }
  D3D9State* Direct3DDevice9Ex::GetEditState() {
    if (m_stateBlock != nullptr)
      return m_stateBlock->GetState();

    return m_state;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::BeginStateBlock() {
    CriticalSection cs(this);

    if (m_stateBlock != nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "BeginStateBlock: attempted to begin a state block when one is already being recorded.");

    m_stateBlock = new Direct3DStateBlock9(this, 0);
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::EndStateBlock(IDirect3DStateBlock9** ppSB) {
    CriticalSection cs(this);
    InitReturnPtr(ppSB);

    if (ppSB == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "EndStateBlock: ppSB was nullptrptr.");

    if (m_stateBlock == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "EndStateBlock: no state block bound.");

    *ppSB = ref(m_stateBlock);
    m_stateBlock = nullptr;

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::SetClipStatus");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetClipStatus(D3DCLIPSTATUS9* pClipStatus) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::GetClipStatus");
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
    CriticalSection cs(this);
    return m_state->GetTexture(Stage, ppTexture);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
    CriticalSection cs(this);
    return GetEditState()->SetTexture(Stage, pTexture);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
    CriticalSection cs(this);
    return m_state->GetTextureStageState(Stage, Type, pValue);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
    CriticalSection cs(this);
    return GetEditState()->SetTextureStageState(Stage, Type, Value);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
    CriticalSection cs(this);
    return m_state->GetSamplerState(Sampler, Type, pValue);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
    CriticalSection cs(this);
    return GetEditState()->SetSamplerState(Sampler, Type, Value);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::ValidateDevice(DWORD* pNumPasses) {
    CriticalSection cs(this);

    if (!pNumPasses)
      return log::d3derr(D3DERR_INVALIDCALL, "ValidateDevice: pNumPasses was nullptr.");

    *pNumPasses = 1;
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::SetPaletteEntries");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::GetPaletteEntries");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetCurrentTexturePalette(UINT PaletteNumber) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::SetCurrentTexturePalette");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetCurrentTexturePalette(UINT *PaletteNumber) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::GetCurrentTexturePalette");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetScissorRect(CONST RECT* pRect) {
    CriticalSection cs(this);
    return GetEditState()->SetScissorRect(pRect);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetScissorRect(RECT* pRect) {
    CriticalSection cs(this);
    return m_state->GetScissorRect(pRect);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetSoftwareVertexProcessing(BOOL bSoftware) {
    CriticalSection cs(this);

    m_softwareVertexProcessing = bSoftware;
    return D3D_OK;
  }
  BOOL    STDMETHODCALLTYPE Direct3DDevice9Ex::GetSoftwareVertexProcessing() {
    CriticalSection cs(this);

    return m_softwareVertexProcessing;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetNPatchMode(float nSegments) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::SetNPatchMode");
    return D3D_OK;
  }
  float   STDMETHODCALLTYPE Direct3DDevice9Ex::GetNPatchMode() {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::GetNPatchMode");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
    CriticalSection cs(this);
    return m_renderer->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
    CriticalSection cs(this);
    return m_renderer->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
    CriticalSection cs(this);
    return m_renderer->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
    CriticalSection cs(this);
    return m_renderer->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::ProcessVertices");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) {
    CriticalSection cs(this);

    InitReturnPtr(ppDecl);

    if (ppDecl == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "CreateVertexDeclaration: ppDecl was nullptr.");

    if (pVertexElements == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "CreateVertexDeclaration: pVertexElements was nullptr.");

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
    std::vector<D3DVERTEXELEMENT9> d3d9Elements;

    size_t count;
    {
      const D3DVERTEXELEMENT9* counter = pVertexElements;
      while (counter->Stream != 0xff)
        counter++;

      count = counter - pVertexElements;
    }

    d3d9Elements.resize(count);
    inputElements.reserve(count);

    std::memcpy(&d3d9Elements[0], pVertexElements, sizeof(D3DVERTEXELEMENT9) * count);

    for (size_t i = 0; i < count; i++) {
      D3D11_INPUT_ELEMENT_DESC desc;
      
      desc.SemanticName = convert::declUsage(true, false, (D3DDECLUSAGE)pVertexElements[i].Usage).c_str();
      desc.SemanticIndex = pVertexElements[i].UsageIndex;
      desc.Format = convert::declType((D3DDECLTYPE)pVertexElements[i].Type);
      desc.InputSlot = pVertexElements[i].Stream;
      desc.AlignedByteOffset = pVertexElements[i].Offset;
      desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
      desc.InstanceDataStepRate = 0;
      
      inputElements.push_back(desc);
    }

    *ppDecl = ref(new Direct3DVertexDeclaration9(this, inputElements, d3d9Elements));

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {
    CriticalSection cs(this);
    return GetEditState()->SetVertexDeclaration(pDecl);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {
    CriticalSection cs(this);
    return m_state->GetVertexDeclaration(ppDecl);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetFVF(DWORD FVF) {
    CriticalSection cs(this);

    m_fvf = FVF;
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetFVF(DWORD* pFVF) {
    CriticalSection cs(this);

    if (pFVF == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetFVF: pFVF was nullptr");

    *pFVF = m_fvf;
    return D3D_OK;
  }

  static int32_t shaderNums[2] = { 0, 0 };

  template <bool Vertex, bool D3D9>
  void DoShaderDump(const uint32_t* func, uint32_t size, const char* type) {
    uint32_t thisShaderNum = shaderNums[Vertex ? 0 : 1];

    const char* shaderDumpPath = "shaderdump";
    CreateDirectoryA(shaderDumpPath, nullptr);

    char dxbcName[64];
    snprintf(dxbcName, 64, Vertex ? "%s/vs_%d.%s" : "%s/ps_%d.%s", shaderDumpPath, thisShaderNum, type);

    FILE* file = fopen(dxbcName, "wb");
    fwrite(func, 1, size, file);
    fclose(file);

    // Disassemble.

    char comments[2048];
    Com<ID3DBlob> blob;

    HRESULT result = D3DERR_INVALIDCALL;

    if (!D3D9) {
      if (!d3dcompiler::disassemble(&result, func, size, D3D_DISASM_ENABLE_COLOR_CODE, comments, &blob))
        log::warn("Failed to load d3dcompiler module for disassembly.");
    }
    else {
      if (!d3dx::dissasembleShader(&result, func, true, comments, &blob))
        log::warn("Failed to load d3dx9 module for disassembly.");
    }

    if (FAILED(result))
      log::warn("Failed to disassemble generated shader!");

    if (blob != nullptr) {
      snprintf(dxbcName, 64, Vertex ? "%s/vs_%d.%s.html" : "%s/ps_%d.%s.html", shaderDumpPath, thisShaderNum, type);

      FILE* file = fopen(dxbcName, "wb");
      fwrite(blob->GetBufferPointer(), 1, blob->GetBufferSize(), file);
      fclose(file);
    }
  }

  template <bool Vertex, typename ID3D9, typename D3D9, typename D3D11>
  HRESULT CreateShader(CONST DWORD* pFunction, ID3D9** ppShader, ID3D11Device* device, Direct3DDevice9Ex* wrapDevice) {
    InitReturnPtr(ppShader);

    if (pFunction == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "Create%sShader: pFunction was nullptr.", Vertex ? "Vertex" : "Pixel");
    
    if (ppShader == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "Create%sShader: ppShader was nullptr.", Vertex ? "Vertex" : "Pixel");

    shaderNums[Vertex ? 0 : 1]++;

    const uint32_t* bytecodePtr = reinterpret_cast<const uint32_t*>(pFunction);

    if (config::getBool(config::ShaderDump))
      DoShaderDump<Vertex, true>(bytecodePtr, dx9asm::byteCodeLength(bytecodePtr), "dx9asm");

    dx9asm::ShaderBytecode* bytecode = nullptr;
    dx9asm::toDXBC(bytecodePtr, &bytecode);

    if (config::getBool(config::ShaderDump) && bytecode != nullptr)
      DoShaderDump<Vertex, false>((const uint32_t*)bytecode->getBytecode(), bytecode->getByteSize(), "dxbc");

    Com<D3D11> shader;
    HRESULT result = D3DERR_INVALIDCALL;

    if (bytecode != nullptr) {
      if (Vertex)
        result = device->CreateVertexShader(bytecode->getBytecode(), bytecode->getByteSize(), nullptr, (ID3D11VertexShader**)&shader);
      else
        result = device->CreatePixelShader(bytecode->getBytecode(), bytecode->getByteSize(), nullptr, (ID3D11PixelShader**)&shader);
    }

    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "Create%sShader: failed to create D3D11 shader.", Vertex ? "Vertex" : "Pixel");

    *ppShader = ref(new D3D9(shaderNums[Vertex ? 0 : 1], wrapDevice, pFunction, shader.ptr(), bytecode));

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
    CriticalSection cs(this);

    return CreateShader<true, IDirect3DVertexShader9, Direct3DVertexShader9, ID3D11VertexShader>(pFunction, ppShader, m_device.ptr(), this);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetVertexShader(IDirect3DVertexShader9* pShader) {
    CriticalSection cs(this);
    return GetEditState()->SetVertexShader(pShader);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetVertexShader(IDirect3DVertexShader9** ppShader) {
    CriticalSection cs(this);
    return m_state->GetVertexShader(ppShader);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
    CriticalSection cs(this);
    return GetEditState()->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
    CriticalSection cs(this);
    return m_state->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
    CriticalSection cs(this);
    return GetEditState()->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
    CriticalSection cs(this);
    return m_state->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
    CriticalSection cs(this);
    return GetEditState()->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
    CriticalSection cs(this);
    return m_state->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {
    CriticalSection cs(this);
    return GetEditState()->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {
    CriticalSection cs(this);
    return m_state->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::SetStreamSourceFreq");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::GetStreamSourceFreq");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetIndices(IDirect3DIndexBuffer9* pIndexData) {
    CriticalSection cs(this);
    return GetEditState()->SetIndices(pIndexData);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetIndices(IDirect3DIndexBuffer9** ppIndexData) {
    CriticalSection cs(this);
    return m_state->GetIndices(ppIndexData);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
    CriticalSection cs(this);

    return CreateShader<false, IDirect3DPixelShader9, Direct3DPixelShader9, ID3D11PixelShader>(pFunction, ppShader, m_device.ptr(), this);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetPixelShader(IDirect3DPixelShader9* pShader) {
    CriticalSection cs(this);
    return GetEditState()->SetPixelShader(pShader);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetPixelShader(IDirect3DPixelShader9** ppShader) {
    CriticalSection cs(this);
    return m_state->GetPixelShader(ppShader);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
    CriticalSection cs(this);
    return GetEditState()->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
    CriticalSection cs(this);
    return m_state->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
    CriticalSection cs(this);
    return GetEditState()->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
    CriticalSection cs(this);
    return m_state->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
    CriticalSection cs(this);
    return GetEditState()->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
    CriticalSection cs(this);
    return m_state->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* DrawRectPatch) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::DrawRectPatch");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::DrawTriPatch");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::DeletePatch(UINT Handle) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::DeletePatch");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {
    CriticalSection cs(this);

    InitReturnPtr(ppQuery);

    switch (Type) {
    case D3DQUERYTYPE_VCACHE:
    case D3DQUERYTYPE_EVENT:
    case D3DQUERYTYPE_OCCLUSION:
    case D3DQUERYTYPE_TIMESTAMP:
    case D3DQUERYTYPE_TIMESTAMPDISJOINT:
    case D3DQUERYTYPE_TIMESTAMPFREQ:
      break;
    default:
    case D3DQUERYTYPE_RESOURCEMANAGER:
    case D3DQUERYTYPE_VERTEXSTATS:
    case D3DQUERYTYPE_PIPELINETIMINGS:
    case D3DQUERYTYPE_INTERFACETIMINGS:
    case D3DQUERYTYPE_VERTEXTIMINGS:
    case D3DQUERYTYPE_PIXELTIMINGS:
    case D3DQUERYTYPE_BANDWIDTHTIMINGS:
    case D3DQUERYTYPE_CACHEUTILIZATION:
      return log::d3derr(D3DERR_NOTAVAILABLE, "CreateQuery: query type not available (%d).", Type);
    }

    if (ppQuery == nullptr)
      return D3D_OK;

    *ppQuery = ref(new Direct3DQuery9(this, Type));
    return D3D_OK;
  }

  // Ex

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetConvolutionMonoKernel(UINT width, UINT height, float* rows, float* columns) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::SetConvolutionMonoKernel");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::ComposeRects(IDirect3DSurface9* pSrc, IDirect3DSurface9* pDst, IDirect3DVertexBuffer9* pSrcRectDescs, UINT NumRects, IDirect3DVertexBuffer9* pDstRectDescs, D3DCOMPOSERECTSOP Operation, int Xoffset, int Yoffset) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::ComposeRects");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetGPUThreadPriority(INT* pPriority) {
    CriticalSection cs(this);

    if (pPriority == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetGPUThreadPriority: pPriority was nullptr.");

    if (FAILED(m_dxgiDevice->GetGPUThreadPriority(pPriority)))
      return log::d3derr(D3DERR_INVALIDCALL, "GetGPUThreadPriority: IDXGIDevice1::GetGPUThreadPriority failed.");

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetGPUThreadPriority(INT Priority) {
    CriticalSection cs(this);

    if (FAILED(m_dxgiDevice->SetGPUThreadPriority(Priority)))
      return log::d3derr(D3DERR_INVALIDCALL, "SetGPUThreadPriority: IDXGIDevice1::SetGPUThreadPriority failed.");

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::WaitForVBlank(UINT iSwapChain) {
    CriticalSection cs(this);

    Direct3DSwapChain9Ex* swapchain = GetInternalSwapchain(iSwapChain);

    if (swapchain == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "WaitForVBlank: invalid swapchain requested (%d).", iSwapChain);

    swapchain->WaitForVBlank();
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CheckResourceResidency(IDirect3DResource9** pResourceArray, UINT32 NumResources) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::CheckResourceResidency");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetMaximumFrameLatency(UINT MaxLatency) {
    CriticalSection cs(this);

    if (MaxLatency > 16)
      MaxLatency = 16;

    if (FAILED(m_dxgiDevice->SetMaximumFrameLatency(MaxLatency)))
      return log::d3derr(D3DERR_INVALIDCALL, "SetMaximumFrameLatency: IDXGIDevice1::SetMaximumFrameLatency failed.");

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetMaximumFrameLatency(UINT* pMaxLatency) {
    CriticalSection cs(this);

    if (pMaxLatency == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetMaximumFrameLatency: pMaxLatency was nullptr.");

    if (FAILED(m_dxgiDevice->GetMaximumFrameLatency(pMaxLatency)))
      return log::d3derr(D3DERR_INVALIDCALL, "GetMaximumFrameLatency: IDXGIDevice1::GetMaximumFrameLatency failed.");

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CheckDeviceState(HWND hDestinationWindow) {
    CriticalSection cs(this);

    Direct3DSwapChain9Ex* swapchain = GetInternalSwapchain(0);

    if (swapchain == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "CheckDeviceState: invalid swapchain to pass test through to (0).");

    return swapchain->TestSwapchain(hDestinationWindow, true);
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::PresentEx(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags) {
    CriticalSection cs(this);

    // Not sure what swapchain to use here, going with this one ~ Josh
    HRESULT result = GetInternalSwapchain(0)->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);

    m_renderer->handleDepthStencilDiscard();

    if (m_pendingCursorUpdate.update)
      SetCursorPosition(m_pendingCursorUpdate.x, m_pendingCursorUpdate.y, D3DCURSOR_IMMEDIATE_UPDATE);
    
    return result;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateRenderTargetEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::CreateRenderTargetEx");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateOffscreenPlainSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::CreateOffscreenPlainSurfaceEx");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateDepthStencilSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::CreateDepthStencilSurfaceEx");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::ResetEx(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::ResetEx");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetDisplayModeEx(UINT iSwapChain, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) {
    CriticalSection cs(this);

    log::stub("Direct3DDevice9Ex::GetDisplayModeEx");
    return D3D_OK;
  }

  void Direct3DDevice9Ex::GetParent(Direct3D9Ex** parent) {
    *parent = static_cast<Direct3D9Ex*>( ref(m_parent) );
  }
  ID3D11DeviceContext* Direct3DDevice9Ex::GetContext() {
    return m_context.ptr();
  }
  ID3D11Device* Direct3DDevice9Ex::GetD3D11Device() {
    return m_device.ptr();
  }
  
  void Direct3DDevice9Ex::setupFPUFlags() {
#ifdef _MSC_VER
    uint32_t currentWord = 0;
    _controlfp_s(&currentWord, _MCW_EM, _MCW_EM); // mask exceptions

    if (config::getBool(config::RespectPrecision)) {
      _controlfp_s(&currentWord, _PC_24, _MCW_PC); // single precision
      _controlfp_s(&currentWord, _RC_NEAR, _MCW_RC); // round to nearest

      // TODO! Find out if we want to respect this by default.
    }
#else
    if (!config::getBool(config::RespectPrecision))
      log::warn("DXUP_RESPECT_PRECISION ignored on Linux.");

    WORD cw;
    __asm__ volatile ("fnstcw %0" : "=m" (cw));
    cw = (cw & ~0xf3f) | 0x3f;
    __asm__ volatile ("fldcw %0" : : "m" (cw));
#endif
  }

}