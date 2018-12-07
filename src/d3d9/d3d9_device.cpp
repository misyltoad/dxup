#include "d3d9_device.h"
#include "d3d9_interface.h"
#include "d3d9_swapchain.h"
#include "d3d9_surface.h"
#include "d3d9_texture.h"
#include "d3d9_vertexbuffer.h"
#include "d3d9_shaders.h"
#include "../dx9asm/dx9asm_translator.h"
#include "../dx9asm/dx9asm_util.h"
#include "../util/config.h"
#include "../util/d3dcompiler_helpers.h"

namespace dxapex {

  Direct3DDevice9Ex::Direct3DDevice9Ex(DeviceInitData* data)
    : m_adapter(data->adapter)
    , m_device(data->device)
    , m_context(data->context)
    , m_parent(data->parent)
    , m_flags(0)
    , m_creationParameters(*data->creationParameters) 
    , m_deviceType(data->deviceType) {
    if (data->ex)
      m_flags |= DeviceFlag_Ex;

    Reset(data->presentParameters);

    CreateAdditionalSwapChain(data->presentParameters, (IDirect3DSwapChain9**)&m_swapchains[0]);

    Com<IDirect3DSurface9> backbuffer;
    HRESULT result = m_swapchains[0]->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);

    D3DVIEWPORT9 implicitViewport;
    implicitViewport.X = 0;
    implicitViewport.Y = 0;
    implicitViewport.Height = data->presentParameters->BackBufferHeight;
    implicitViewport.Width = data->presentParameters->BackBufferWidth;
    implicitViewport.MinZ = 0.0f;
    implicitViewport.MaxZ = 1.0f;
    SetViewport(&implicitViewport);

    /*D3D11_QUERY_DESC desc;
    desc.MiscFlags = 0;
    desc.Query = D3D11_QUERY::D3D11_QUERY_TIMESTAMP;
    m_device->CreateQuery(&desc, &m_endQuery);*/

    if (!FAILED(result))
      SetRenderTarget(0, backbuffer.ptr());
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::QueryInterface(REFIID riid, LPVOID* ppv) {
    InitReturnPtr(ppv);

    if (ppv == nullptr)
      return E_POINTER;

    if (riid == __uuidof(IDirect3DDevice9Ex) || riid == __uuidof(IDirect3DDevice9) || riid == __uuidof(IUnknown)) {
      *ppv = ref(this);
    }

    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::TestCooperativeLevel() {
    if (m_flags & DeviceFlag_Ex)
      return D3D_OK;

    Direct3DSwapChain9Ex* swapchain = reinterpret_cast<Direct3DSwapChain9Ex*>(m_swapchains[0].ptr());
    return swapchain->TestSwapchain(nullptr, 0);
  }

  UINT    STDMETHODCALLTYPE Direct3DDevice9Ex::GetAvailableTextureMem() {
    DXGI_ADAPTER_DESC adapterDesc; 
    m_adapter->GetDesc(&adapterDesc);
    return UINT(adapterDesc.DedicatedVideoMemory / 1024ul);
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::EvictManagedResources() {
    log::stub("Direct3DDevice9Ex::EvictManagedResources");
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetDirect3D(IDirect3D9** ppD3D9) {
    if (!ppD3D9)
      return D3DERR_INVALIDCALL;
    *ppD3D9 = ref(m_parent);

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetDeviceCaps(D3DCAPS9* pCaps) {

    if (!pCaps)
      return D3DERR_INVALIDCALL;

    // Assuming you're not running a potato if you're using dxapex.
    // Feel free to improve.

    DXGI_ADAPTER_DESC adapterDesc;
    m_adapter->GetDesc(&adapterDesc);
    
    pCaps->DeviceType = m_deviceType;
    pCaps->AdapterOrdinal = 0;
    pCaps->Caps = 0;
    pCaps->Caps2 = D3DCAPS2_CANMANAGERESOURCE | D3DCAPS2_DYNAMICTEXTURES | D3DCAPS2_FULLSCREENGAMMA | D3DCAPS2_CANAUTOGENMIPMAP;
    pCaps->Caps3 = D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD | D3DCAPS3_COPY_TO_VIDMEM | D3DCAPS3_COPY_TO_SYSTEMMEM | D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION;
    pCaps->PresentationIntervals = D3DPRESENT_INTERVAL_DEFAULT | D3DPRESENT_INTERVAL_ONE | D3DPRESENT_INTERVAL_TWO | D3DPRESENT_INTERVAL_THREE | D3DPRESENT_INTERVAL_FOUR | D3DPRESENT_INTERVAL_IMMEDIATE;
    pCaps->CursorCaps = D3DCURSORCAPS_COLOR | D3DCURSORCAPS_LOWRES;
    pCaps->DevCaps = D3DDEVCAPS_CANBLTSYSTONONLOCAL | D3DDEVCAPS_CANRENDERAFTERFLIP | D3DDEVCAPS_DRAWPRIMITIVES2 | D3DDEVCAPS_DRAWPRIMITIVES2EX | D3DDEVCAPS_DRAWPRIMTLVERTEX | D3DDEVCAPS_EXECUTESYSTEMMEMORY | D3DDEVCAPS_EXECUTEVIDEOMEMORY | D3DDEVCAPS_HWRASTERIZATION | D3DDEVCAPS_HWTRANSFORMANDLIGHT | D3DDEVCAPS_PUREDEVICE | D3DDEVCAPS_TEXTURENONLOCALVIDMEM | D3DDEVCAPS_TEXTUREVIDEOMEMORY | D3DDEVCAPS_TLVERTEXSYSTEMMEMORY | D3DDEVCAPS_TLVERTEXVIDEOMEMORY;
    pCaps->PrimitiveMiscCaps = D3DPMISCCAPS_MASKZ | D3DPMISCCAPS_CULLNONE | D3DPMISCCAPS_CULLCW | D3DPMISCCAPS_CULLCCW | D3DPMISCCAPS_COLORWRITEENABLE | D3DPMISCCAPS_CLIPPLANESCALEDPOINTS | D3DPMISCCAPS_TSSARGTEMP | D3DPMISCCAPS_BLENDOP | D3DPMISCCAPS_INDEPENDENTWRITEMASKS | D3DPMISCCAPS_FOGANDSPECULARALPHA | D3DPMISCCAPS_SEPARATEALPHABLEND | D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS | D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING | D3DPMISCCAPS_FOGVERTEXCLAMPED;
    pCaps->RasterCaps = D3DPRASTERCAPS_ANISOTROPY | D3DPRASTERCAPS_COLORPERSPECTIVE | D3DPRASTERCAPS_DITHER | D3DPRASTERCAPS_DEPTHBIAS | D3DPRASTERCAPS_FOGRANGE | D3DPRASTERCAPS_FOGTABLE | D3DPRASTERCAPS_FOGVERTEX | D3DPRASTERCAPS_MIPMAPLODBIAS | D3DPRASTERCAPS_MULTISAMPLE_TOGGLE | D3DPRASTERCAPS_SCISSORTEST | D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS | D3DPRASTERCAPS_WFOG | D3DPRASTERCAPS_ZFOG | D3DPRASTERCAPS_ZTEST;
    pCaps->ZCmpCaps = D3DPCMPCAPS_NEVER | D3DPCMPCAPS_LESS | D3DPCMPCAPS_EQUAL | D3DPCMPCAPS_LESSEQUAL | D3DPCMPCAPS_GREATER | D3DPCMPCAPS_NOTEQUAL | D3DPCMPCAPS_GREATEREQUAL | D3DPCMPCAPS_ALWAYS;
    pCaps->SrcBlendCaps = D3DPBLENDCAPS_ZERO | D3DPBLENDCAPS_ONE | D3DPBLENDCAPS_SRCCOLOR | D3DPBLENDCAPS_INVSRCCOLOR | D3DPBLENDCAPS_SRCALPHA | D3DPBLENDCAPS_INVSRCALPHA | D3DPBLENDCAPS_DESTALPHA | D3DPBLENDCAPS_INVDESTALPHA | D3DPBLENDCAPS_DESTCOLOR | D3DPBLENDCAPS_INVDESTCOLOR | D3DPBLENDCAPS_SRCALPHASAT | D3DPBLENDCAPS_BOTHSRCALPHA | D3DPBLENDCAPS_BOTHINVSRCALPHA | D3DPBLENDCAPS_BLENDFACTOR | D3DPBLENDCAPS_INVSRCCOLOR2 | D3DPBLENDCAPS_SRCCOLOR2;
    pCaps->DestBlendCaps = pCaps->SrcBlendCaps;
    pCaps->AlphaCmpCaps = D3DPCMPCAPS_NEVER | D3DPCMPCAPS_LESS | D3DPCMPCAPS_EQUAL | D3DPCMPCAPS_LESSEQUAL | D3DPCMPCAPS_GREATER | D3DPCMPCAPS_NOTEQUAL | D3DPCMPCAPS_GREATEREQUAL | D3DPCMPCAPS_ALWAYS;
    pCaps->ShadeCaps = D3DPSHADECAPS_COLORGOURAUDRGB | D3DPSHADECAPS_SPECULARGOURAUDRGB | D3DPSHADECAPS_ALPHAGOURAUDBLEND | D3DPSHADECAPS_FOGGOURAUD;
    pCaps->TextureCaps = D3DPTEXTURECAPS_ALPHA | D3DPTEXTURECAPS_ALPHAPALETTE | D3DPTEXTURECAPS_PERSPECTIVE | D3DPTEXTURECAPS_PROJECTED | D3DPTEXTURECAPS_CUBEMAP | D3DPTEXTURECAPS_VOLUMEMAP | D3DPTEXTURECAPS_POW2 | D3DPTEXTURECAPS_NONPOW2CONDITIONAL | D3DPTEXTURECAPS_CUBEMAP_POW2 | D3DPTEXTURECAPS_VOLUMEMAP_POW2 | D3DPTEXTURECAPS_MIPMAP | D3DPTEXTURECAPS_MIPVOLUMEMAP | D3DPTEXTURECAPS_MIPCUBEMAP;
    pCaps->TextureFilterCaps = D3DPTFILTERCAPS_MINFPOINT | D3DPTFILTERCAPS_MINFLINEAR | D3DPTFILTERCAPS_MINFANISOTROPIC | D3DPTFILTERCAPS_MIPFPOINT | D3DPTFILTERCAPS_MIPFLINEAR | D3DPTFILTERCAPS_MAGFPOINT | D3DPTFILTERCAPS_MAGFLINEAR | D3DPTFILTERCAPS_MAGFANISOTROPIC;
    pCaps->CubeTextureFilterCaps = pCaps->TextureFilterCaps;
    pCaps->VolumeTextureFilterCaps = pCaps->TextureFilterCaps;
    pCaps->TextureAddressCaps = D3DPTADDRESSCAPS_BORDER | D3DPTADDRESSCAPS_INDEPENDENTUV | D3DPTADDRESSCAPS_WRAP | D3DPTADDRESSCAPS_MIRROR | D3DPTADDRESSCAPS_CLAMP | D3DPTADDRESSCAPS_MIRRORONCE;
    pCaps->VolumeTextureAddressCaps = pCaps->TextureAddressCaps;
    pCaps->LineCaps = D3DLINECAPS_ALPHACMP | D3DLINECAPS_BLEND | D3DLINECAPS_TEXTURE | D3DLINECAPS_ZTEST | D3DLINECAPS_FOG;
    pCaps->MaxTextureWidth = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    pCaps->MaxTextureHeight = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    pCaps->MaxVolumeExtent = D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
    pCaps->MaxTextureRepeat = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    pCaps->MaxTextureAspectRatio = pCaps->MaxTextureWidth;
    pCaps->MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
    pCaps->MaxVertexW = 1e10f;
    pCaps->GuardBandLeft = -1e9f;
    pCaps->GuardBandTop = -1e9f;
    pCaps->GuardBandRight = 1e9f;
    pCaps->GuardBandBottom = 1e9f;
    pCaps->ExtentsAdjust = 0.0f;
    pCaps->StencilCaps = D3DSTENCILCAPS_KEEP | D3DSTENCILCAPS_ZERO | D3DSTENCILCAPS_REPLACE | D3DSTENCILCAPS_INCRSAT | D3DSTENCILCAPS_DECRSAT | D3DSTENCILCAPS_INVERT | D3DSTENCILCAPS_INCR | D3DSTENCILCAPS_DECR | D3DSTENCILCAPS_TWOSIDED;
    pCaps->FVFCaps = D3DFVFCAPS_PSIZE;
    pCaps->TextureOpCaps = D3DTEXOPCAPS_DISABLE | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2 | D3DTEXOPCAPS_MODULATE | D3DTEXOPCAPS_MODULATE2X | D3DTEXOPCAPS_MODULATE4X | D3DTEXOPCAPS_ADD | D3DTEXOPCAPS_ADDSIGNED | D3DTEXOPCAPS_ADDSIGNED2X | D3DTEXOPCAPS_SUBTRACT | D3DTEXOPCAPS_ADDSMOOTH | D3DTEXOPCAPS_BLENDDIFFUSEALPHA | D3DTEXOPCAPS_BLENDTEXTUREALPHA | D3DTEXOPCAPS_BLENDFACTORALPHA | D3DTEXOPCAPS_BLENDTEXTUREALPHAPM | D3DTEXOPCAPS_BLENDCURRENTALPHA | D3DTEXOPCAPS_PREMODULATE | D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR | D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA | D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR | D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA | D3DTEXOPCAPS_BUMPENVMAP | D3DTEXOPCAPS_BUMPENVMAPLUMINANCE | D3DTEXOPCAPS_DOTPRODUCT3 | D3DTEXOPCAPS_MULTIPLYADD | D3DTEXOPCAPS_LERP;
    pCaps->MaxTextureBlendStages = D3D11_REQ_BLEND_OBJECT_COUNT_PER_DEVICE;

    pCaps->MaxSimultaneousTextures = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;

    pCaps->VertexProcessingCaps = D3DVTXPCAPS_TEXGEN | D3DVTXPCAPS_MATERIALSOURCE7 | D3DVTXPCAPS_DIRECTIONALLIGHTS | D3DVTXPCAPS_POSITIONALLIGHTS | D3DVTXPCAPS_LOCALVIEWER | D3DVTXPCAPS_TWEENING;
    pCaps->MaxActiveLights = 8;
    pCaps->MaxUserClipPlanes = 8;
    pCaps->MaxVertexBlendMatrices = 4;
    pCaps->MaxVertexBlendMatrixIndex = 7;
    pCaps->MaxPointSize = 1.0f;
    pCaps->MaxPrimitiveCount = 0xFFFFFFFF;
    pCaps->MaxVertexIndex = 0xFFFFFFFF;
    pCaps->MaxStreams = 1024;
    pCaps->MaxStreamStride = 4096;
    //pCaps->VertexShaderVersion = D3DVS_VERSION(3, 0);
    pCaps->VertexShaderVersion = D3DVS_VERSION(1, 1); // Only sm1_1 support for now!
    pCaps->MaxVertexShaderConst = 256;
    //pCaps->PixelShaderVersion = D3DPS_VERSION(3, 0);
    pCaps->PixelShaderVersion = D3DPS_VERSION(1, 1);
    pCaps->PixelShader1xMaxValue = 65504.f;
    pCaps->DevCaps2 = D3DDEVCAPS2_STREAMOFFSET | D3DDEVCAPS2_VERTEXELEMENTSCANSHARESTREAMOFFSET | D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES;
    pCaps->MasterAdapterOrdinal = 0;
    pCaps->AdapterOrdinalInGroup = 0;
    pCaps->NumberOfAdaptersInGroup = 1;
    pCaps->DeclTypes = D3DDTCAPS_UBYTE4 | D3DDTCAPS_UBYTE4N | D3DDTCAPS_SHORT2N | D3DDTCAPS_SHORT4N | D3DDTCAPS_USHORT2N | D3DDTCAPS_USHORT4N | D3DDTCAPS_UDEC3 | D3DDTCAPS_DEC3N | D3DDTCAPS_FLOAT16_2 | D3DDTCAPS_FLOAT16_4;
    pCaps->NumSimultaneousRTs = 4;
    pCaps->StretchRectFilterCaps = D3DPTFILTERCAPS_MINFPOINT | D3DPTFILTERCAPS_MINFLINEAR | D3DPTFILTERCAPS_MAGFPOINT | D3DPTFILTERCAPS_MAGFLINEAR;
    pCaps->VS20Caps.Caps = D3DVS20CAPS_PREDICATION;
    pCaps->VS20Caps.DynamicFlowControlDepth = 24;
    pCaps->VS20Caps.StaticFlowControlDepth = 4;
    pCaps->VS20Caps.NumTemps = 32;
    pCaps->PS20Caps.Caps = D3DPS20CAPS_ARBITRARYSWIZZLE | D3DPS20CAPS_GRADIENTINSTRUCTIONS | D3DPS20CAPS_PREDICATION;
    pCaps->PS20Caps.DynamicFlowControlDepth = 24;
    pCaps->PS20Caps.StaticFlowControlDepth = 4;
    pCaps->PS20Caps.NumTemps = 32;
    pCaps->VertexTextureFilterCaps = pCaps->TextureFilterCaps;
    pCaps->MaxVertexShader30InstructionSlots = 32768;
    pCaps->MaxPixelShader30InstructionSlots = 32768;
    pCaps->MaxVShaderInstructionsExecuted = 65535;
    pCaps->MaxPShaderInstructionsExecuted = 65535;

    pCaps->MaxNpatchTessellationLevel = 0.0f;
    pCaps->Reserved5 = 0;

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {
    log::stub("Direct3DDevice9Ex::GetDisplayMode");

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) {
    if (!pParameters)
      return D3DERR_INVALIDCALL;

    *pParameters = m_creationParameters;

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {
    log::stub("Direct3DDevice9Ex::SetCursorProperties");
    return D3D_OK;
  }
  void    STDMETHODCALLTYPE Direct3DDevice9Ex::SetCursorPosition(int X, int Y, DWORD Flags) {
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

    ::ShowCursor(bShow);

    return TRUE;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** ppSwapChain) {
    InitReturnPtr(ppSwapChain);

    if (!ppSwapChain)
      return D3DERR_INVALIDCALL;

    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    memset(&SwapChainDesc, 0, sizeof(SwapChainDesc));

    UINT BackBufferCount = pPresentationParameters->BackBufferCount;
    if (BackBufferCount == 0)
      BackBufferCount = 1;

    SwapChainDesc.BufferCount = BackBufferCount + 1;
    SwapChainDesc.BufferDesc.Width = pPresentationParameters->BackBufferWidth;
    SwapChainDesc.BufferDesc.Height = pPresentationParameters->BackBufferHeight;
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
    SwapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.OutputWindow = m_creationParameters.hFocusWindow;
    SwapChainDesc.Windowed = true;
    SwapChainDesc.SampleDesc.Count = (UINT)pPresentationParameters->MultiSampleType;

    if (SwapChainDesc.SampleDesc.Count == 0)
      SwapChainDesc.SampleDesc.Count = 1;

    SwapChainDesc.SampleDesc.Quality = pPresentationParameters->MultiSampleQuality;

    Com<Direct3D9Ex> parent;
    GetParent(&parent);

    Com<IDXGISwapChain> dxgiSwapChain;
    Com<IDXGIFactory> dxgiFactory;

    parent->GetDXGIFactory(&dxgiFactory);
    HRESULT result = dxgiFactory->CreateSwapChain(m_device.ptr(), &SwapChainDesc, &dxgiSwapChain);

    if (FAILED(result))
      return result;

    for (size_t i = 0; i < m_swapchains.size(); i++)
    {
      if (m_swapchains[i] == nullptr) {
        m_swapchains[i] = ref( new Direct3DSwapChain9Ex(this, pPresentationParameters, dxgiSwapChain.ptr()) );

        *ppSwapChain = ref(m_swapchains[i]);
        return D3D_OK;
      }
    }

    return D3DERR_INVALIDCALL;;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {
    InitReturnPtr(pSwapChain);
    if (!pSwapChain || iSwapChain > m_swapchains.size() || m_swapchains[iSwapChain] == nullptr)
      return D3DERR_INVALIDCALL;

    *pSwapChain = ref(m_swapchains[iSwapChain]);

    return D3D_OK;
  }
  UINT    STDMETHODCALLTYPE Direct3DDevice9Ex::GetNumberOfSwapChains() {
    UINT swapchainCount = 0;

    for (size_t i = 0; i < m_swapchains.size(); i++) {
      if (m_swapchains[i] == nullptr)
        swapchainCount++;
    }
    return swapchainCount;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {


    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
    return PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, 0);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
    if (iSwapChain > m_swapchains.size() || m_swapchains[iSwapChain] == nullptr)
      return D3DERR_INVALIDCALL;

    return m_swapchains[iSwapChain]->GetBackBuffer(iBackBuffer, Type, ppBackBuffer);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {
    if (iSwapChain > m_swapchains.size() || m_swapchains[iSwapChain] == nullptr)
      return D3DERR_INVALIDCALL;

    return m_swapchains[iSwapChain]->GetRasterStatus(pRasterStatus);
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetDialogBoxMode(BOOL bEnableDialogs) {
    log::stub("Direct3DDevice9Ex::SetDialogBoxMode");
    return D3D_OK;
  }
  void    STDMETHODCALLTYPE Direct3DDevice9Ex::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
    log::stub("Direct3DDevice9Ex::SetGammaRamp");
  }
  void    STDMETHODCALLTYPE Direct3DDevice9Ex::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {
    log::stub("Direct3DDevice9Ex::GetGammaRamp");
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
    
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = Width;
    desc.Height = Height;
    desc.Format = convert::format(Format);
    desc.Usage = convert::usage(Pool, Usage);
    desc.CPUAccessFlags = convert::cpuFlags(Pool, Usage);
    desc.MipLevels = desc.Usage & D3DUSAGE_DYNAMIC ? 1 : 0;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.MiscFlags = desc.Usage & D3DUSAGE_DYNAMIC ? D3D11_RESOURCE_MISC_GDI_COMPATIBLE : D3D11_RESOURCE_MISC_GENERATE_MIPS;

    Com<ID3D11Texture2D> texture;
    HRESULT result = m_device->CreateTexture2D(&desc, nullptr, &texture);

    if (FAILED(result))
      return D3DERR_INVALIDCALL;

    *ppTexture = ref(new Direct3DTexture9(this, texture.ptr(), Pool, Usage));

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {
    log::stub("Direct3DDevice9Ex::CreateVolumeTexture");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {
    log::stub("Direct3DDevice9Ex::CreateCubeTexture");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {
    InitReturnPtr(ppVertexBuffer);
    InitReturnPtr(pSharedHandle);

    if (!ppVertexBuffer)
      return D3DERR_INVALIDCALL;

    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = Length;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = convert::cpuFlags(Pool, Usage);
    desc.Usage = convert::usage(Pool, Usage);
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    Com<ID3D11Buffer> buffer;
    HRESULT result = m_device->CreateBuffer(&desc, nullptr, &buffer);
    if (FAILED(result))
      return D3DERR_INVALIDCALL;

    *ppVertexBuffer = ref(new Direct3DVertexBuffer9(this, buffer.ptr(), Pool, FVF, Usage));

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {
    log::stub("Direct3DDevice9Ex::CreateIndexBuffer");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
    log::stub("Direct3DDevice9Ex::CreateRenderTarget");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
    log::stub("Direct3DDevice9Ex::CreateDepthStencilSurface");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {

    RECT destRect;
    destRect.left = pDestPoint ? pDestPoint->x : 0;
    destRect.top = pDestPoint ? pDestPoint->y : 0;

    StretchRect(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint ? &destRect : nullptr, D3DTEXF_NONE);
    
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) {
    if (pSourceTexture == nullptr || pDestinationTexture == nullptr)
      return D3DERR_INVALIDCALL;

    switch (pSourceTexture->GetType()) {

    case D3DRTYPE_TEXTURE: {

      IDirect3DTexture9* srcTex = (Direct3DTexture9*)pSourceTexture;
      IDirect3DTexture9* dstTex = (Direct3DTexture9*)pDestinationTexture;

      if (srcTex == nullptr || dstTex == nullptr)
        return D3DERR_INVALIDCALL;

      Com<IDirect3DSurface9> srcSurface;
      Com<IDirect3DSurface9> dstSurface;

      srcTex->GetSurfaceLevel(0, &srcSurface);
      dstTex->GetSurfaceLevel(0, &dstSurface);

      StretchRect(srcSurface.ptr(), NULL, dstSurface.ptr(), NULL, D3DTEXF_NONE);
      dstTex->GenerateMipSubLevels();

      return D3D_OK;

    }

    default: return D3DERR_INVALIDCALL;
    }
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {
    StretchRect(pRenderTarget, NULL, pDestSurface, NULL, D3DTEXF_NONE);
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
    if (iSwapChain >= m_swapchains.size() || m_swapchains[iSwapChain] == nullptr)
      return D3DERR_INVALIDCALL;

    m_swapchains[iSwapChain]->GetFrontBufferData(pDestSurface);  
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {

    Direct3DSurface9* src = reinterpret_cast<Direct3DSurface9*>(pSourceSurface);
    Com<ID3D11Texture2D> srcTex;
    src->GetD3D11Texture(&srcTex);

    Direct3DSurface9* dest = reinterpret_cast<Direct3DSurface9*>(pDestSurface);
    Com<ID3D11Texture2D> destTex;
    dest->GetD3D11Texture(&destTex);

    if (pSourceRect != nullptr && pDestRect != nullptr) {
      UINT x = pDestRect->left;
      UINT y = pDestRect->top;

      // This doesn't properly handle 'stretching'.
      D3D11_BOX box;
      box.left = pSourceRect->left;
      box.right = pSourceRect->right;
      box.top = pSourceRect->top;
      box.bottom = pSourceRect->bottom;
      box.top = 0;
      box.back = 0;
      m_context->CopySubresourceRegion(destTex.ptr(), dest->GetSubresource(), x, y, 0, srcTex.ptr(), src->GetSubresource(), &box);

      return D3D_OK;
    }

    m_context->CopySubresourceRegion(destTex.ptr(), dest->GetSubresource(), 0, 0, 0, srcTex.ptr(), src->GetSubresource(), nullptr);
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {
    /*if (pSurface == nullptr)
      return D3DERR_INVALIDCALL;

    D3DLOCKED_RECT lockedRect;
    HRESULT result = pSurface->LockRect(&lockedRect, pRect, 0);
    if (FAILED(result))
      return D3DERR_INVALIDCALL;

    D3DSURFACE_DESC desc;
    pSurface->GetDesc(&desc);

    UINT rows = pRect ? pRect->bottom - pRect->top : desc.Height;
    UINT width = pRect ? pRect->right - pRect->left : desc.Width;

    for (int y = 0; y < rows; y++) {
      for (int x = 0; x < width; x++) {
        lockedRect.pBits[(y * width) + x] = 
      }
    }*/

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
    log::stub("Direct3DDevice9Ex::CreateOffscreenPlainSurface");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
    
    Direct3DSurface9* surface = (Direct3DSurface9*)pRenderTarget;
    Com<ID3D11RenderTargetView> rtv;
    surface->GetD3D11RenderTarget(&rtv);

    if (rtv == nullptr)
      log::warn("No RTV for surface we want as RT");

    m_context->OMSetRenderTargets(1, &rtv, nullptr);

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
    log::stub("Direct3DDevice9Ex::GetRenderTarget");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {
    log::stub("Direct3DDevice9Ex::SetDepthStencilSurface");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
    log::stub("Direct3DDevice9Ex::GetDepthStencilSurface");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::BeginScene() {
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::EndScene() {
    m_context->Flush();
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
    Com<IDirect3DSurface9> d3d9Surface;
    m_swapchains[0]->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &d3d9Surface);
    Direct3DSurface9* surface = reinterpret_cast<Direct3DSurface9*>(d3d9Surface.ptr());

    Com<ID3D11RenderTargetView> renderTarget;
    surface->GetD3D11RenderTarget(&renderTarget);

    FLOAT color[4];
    convert::color(Color, color);

    m_context->ClearRenderTargetView(renderTarget.ptr(), color);
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
    log::stub("Direct3DDevice9Ex::SetTransform");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
    log::stub("Direct3DDevice9Ex::GetTransform");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::MultiplyTransform(D3DTRANSFORMSTATETYPE TransformState, CONST D3DMATRIX* pMatrix) {
    log::stub("Direct3DDevice9Ex::MultiplyTransform");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetViewport(CONST D3DVIEWPORT9* pViewport) {
    if (!pViewport)
      return D3DERR_INVALIDCALL;

    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = (FLOAT) pViewport->X;
    viewport.TopLeftY = (FLOAT) pViewport->Y;
    viewport.MinDepth = pViewport->MinZ;
    viewport.MaxDepth = pViewport->MaxZ;
    viewport.Width = (FLOAT)pViewport->Width;
    viewport.Height = (FLOAT)pViewport->Height;
    m_context->RSSetViewports(1, &viewport);

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetViewport(D3DVIEWPORT9* pViewport) {
    if (!pViewport)
      return D3DERR_INVALIDCALL;

    D3D11_VIEWPORT viewport;
    UINT numViewports = 1;
    m_context->RSGetViewports(&numViewports, &viewport);

    pViewport->MaxZ = viewport.MaxDepth;
    pViewport->MinZ = viewport.MinDepth;
    pViewport->Width = (DWORD) viewport.Width;
    pViewport->Height = (DWORD) viewport.Height;
    pViewport->X = (DWORD) viewport.TopLeftX;
    pViewport->Y = (DWORD) viewport.TopLeftY;

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetMaterial(CONST D3DMATERIAL9* pMaterial) {
    log::stub("Direct3DDevice9Ex::SetMaterial");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetMaterial(D3DMATERIAL9* pMaterial) {
    log::stub("Direct3DDevice9Ex::GetMaterial");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetLight(DWORD Index, CONST D3DLIGHT9* pLight) {
    log::stub("Direct3DDevice9Ex::SetLight");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetLight(DWORD Index, D3DLIGHT9* pLight) {
    log::stub("Direct3DDevice9Ex::GetLight");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::LightEnable(DWORD Index, BOOL Enable) {
    log::stub("Direct3DDevice9Ex::LightEnable");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetLightEnable(DWORD Index, BOOL* pEnable) {
    log::stub("Direct3DDevice9Ex::GetLightEnable");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetClipPlane(DWORD Index, CONST float* pPlane) {
    log::stub("Direct3DDevice9Ex::SetClipPlane");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetClipPlane(DWORD Index, float* pPlane) {
    log::stub("Direct3DDevice9Ex::GetClipPlane");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
    log::stub("Direct3DDevice9Ex::SetRenderState");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {
    log::stub("Direct3DDevice9Ex::GetRenderState");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {
    log::stub("Direct3DDevice9Ex::CreateStateBlock");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::BeginStateBlock() {
    log::stub("Direct3DDevice9Ex::BeginStateBlock");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::EndStateBlock(IDirect3DStateBlock9** ppSB) {
    log::stub("Direct3DDevice9Ex::EndStateBlock");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) {
    log::stub("Direct3DDevice9Ex::SetClipStatus");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetClipStatus(D3DCLIPSTATUS9* pClipStatus) {
    log::stub("Direct3DDevice9Ex::GetClipStatus");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
    log::stub("Direct3DDevice9Ex::GetTexture");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
    log::stub("Direct3DDevice9Ex::SetTexture");

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
    log::stub("Direct3DDevice9Ex::GetTextureStageState");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
    log::stub("Direct3DDevice9Ex::SetTextureStageState");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
    log::stub("Direct3DDevice9Ex::GetSamplerState");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
    log::stub("Direct3DDevice9Ex::SetSamplerState");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::ValidateDevice(DWORD* pNumPasses) {
    if (!pNumPasses)
      return D3DERR_INVALIDCALL;

    *pNumPasses = 1;
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {
    log::stub("Direct3DDevice9Ex::SetPaletteEntries");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) {
    log::stub("Direct3DDevice9Ex::GetPaletteEntries");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetCurrentTexturePalette(UINT PaletteNumber) {
    log::stub("Direct3DDevice9Ex::SetCurrentTexturePalette");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetCurrentTexturePalette(UINT *PaletteNumber) {
    log::stub("Direct3DDevice9Ex::GetCurrentTexturePalette");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetScissorRect(CONST RECT* pRect) {
    if (pRect == nullptr)
      return D3DERR_INVALIDCALL;

    D3D11_RECT rect;
    rect.bottom = pRect->bottom;
    rect.left = pRect->left;
    rect.right = pRect->right;
    rect.top = pRect->top;

    m_context->RSSetScissorRects(1, &rect);
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetScissorRect(RECT* pRect) {
    if (pRect == nullptr)
      return D3DERR_INVALIDCALL;

    D3D11_RECT rect;
    UINT rects = 1;
    m_context->RSGetScissorRects(&rects, &rect);

    pRect->bottom = rect.bottom;
    pRect->left = rect.left;
    pRect->right = rect.right;
    pRect->top = rect.top;

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetSoftwareVertexProcessing(BOOL bSoftware) {
    m_softwareVertexProcessing = bSoftware;
    return D3D_OK;
  }
  BOOL    STDMETHODCALLTYPE Direct3DDevice9Ex::GetSoftwareVertexProcessing() {
    return m_softwareVertexProcessing;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetNPatchMode(float nSegments) {
    log::stub("Direct3DDevice9Ex::SetNPatchMode");
    return D3D_OK;
  }
  float   STDMETHODCALLTYPE Direct3DDevice9Ex::GetNPatchMode() {
    log::stub("Direct3DDevice9Ex::GetNPatchMode");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
    D3D_PRIMITIVE_TOPOLOGY topology;
    UINT drawCount = convert::primitiveData(PrimitiveType, PrimitiveCount, topology);

    m_context->IASetPrimitiveTopology(topology);
    m_context->Draw(drawCount, StartVertex);

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
    log::stub("Direct3DDevice9Ex::DrawIndexedPrimitive");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
    log::stub("Direct3DDevice9Ex::DrawPrimitiveUP");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
    log::stub("Direct3DDevice9Ex::DrawIndexedPrimitiveUP");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {
    log::stub("Direct3DDevice9Ex::ProcessVertices");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) {
    InitReturnPtr(ppDecl);

    if (ppDecl == nullptr || pVertexElements == nullptr)
      return D3DERR_INVALIDCALL;

    D3DVERTEXELEMENT9 lastElement = D3DDECL_END();

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;

    auto vertexElementEqual = [] (const D3DVERTEXELEMENT9& a, const D3DVERTEXELEMENT9& b) {
      return  a.Method == b.Method &&
              a.Offset == b.Offset &&
              a.Stream == b.Stream &&
              a.Type == b.Type &&
              a.Usage == b.Usage &&
              a.UsageIndex == b.UsageIndex;
    };

    while (!vertexElementEqual(*pVertexElements, lastElement)) {
      D3D11_INPUT_ELEMENT_DESC desc = { 0 };
      
      desc.SemanticName = convert::declUsage((D3DDECLUSAGE)pVertexElements->Usage).c_str();
      desc.Format = convert::declType((D3DDECLTYPE)pVertexElements->Type);
      desc.AlignedByteOffset = pVertexElements->Offset;
      desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
      desc.InputSlot = pVertexElements->UsageIndex;
      
      inputElements.push_back(desc);

      pVertexElements++;
    }


    //Com<ID3D11InputLayout> layout;
    //m_device->CreateInputLayout(&inputElements[0], inputElements.size(), nullptr, 0, &layout);

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {
    log::stub("Direct3DDevice9Ex::SetVertexDeclaration");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {
    log::stub("Direct3DDevice9Ex::GetVertexDeclaration");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetFVF(DWORD FVF) {
    m_fvf = FVF;
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetFVF(DWORD* pFVF) {
    if (pFVF == nullptr)
      return D3DERR_INVALIDCALL;

    *pFVF = m_fvf;
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
    dx9asm::ShaderBytecode* bytecode = nullptr;
    dx9asm::toDXBC((const uint32_t*)pFunction, &bytecode);

    static int32_t shaderNum = 0;
    shaderNum++;

    if (config::getBool(config::ShaderDump)) {

      char name[64];
      snprintf(name, 64, "shader_%d.bin", shaderNum);

      FILE* file = fopen(name, "wb");
      fwrite(bytecode->getBytecode(), 1, bytecode->getByteSize(), file);
      fclose(file);
    }

    Com<ID3D11VertexShader> shader;
    HRESULT result = m_device->CreateVertexShader(bytecode->getBytecode(), bytecode->getByteSize(), nullptr, &shader);
    if (FAILED(result)) {
      log::fail("Shader translation failed!");
      return D3DERR_INVALIDCALL;
    }

    if (config::getBool(config::ShaderDump)) {
      char comments[2048];
      Com<ID3DBlob> blob;

      HRESULT result = S_OK;

      if (!d3dcompiler::disassemble(&result, bytecode->getBytecode(), bytecode->getByteSize(), D3D_DISASM_ENABLE_COLOR_CODE, comments, &blob))
        log::warn("Failed to load d3dcompiler module for disassembly.");

      if (FAILED(result))
        log::warn("Failed to disassemble generated shader!");

      if (blob != nullptr) {

        char name[64];
        snprintf(name, 64, "shader_%d.html", shaderNum);

        FILE* file = fopen(name, "wb");
        fwrite(blob->GetBufferPointer(), 1, blob->GetBufferSize(), file);
        fclose(file);
      }
    }
    
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetVertexShader(IDirect3DVertexShader9* pShader) {
    log::stub("Direct3DDevice9Ex::SetVertexShader");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetVertexShader(IDirect3DVertexShader9** ppShader) {
    log::stub("Direct3DDevice9Ex::GetVertexShader");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
    log::stub("Direct3DDevice9Ex::SetVertexShaderConstantF");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
    log::stub("Direct3DDevice9Ex::GetVertexShaderConstantF");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
    log::stub("Direct3DDevice9Ex::SetVertexShaderConstantI");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
    log::stub("Direct3DDevice9Ex::GetVertexShaderConstantI");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
    log::stub("Direct3DDevice9Ex::SetVertexShaderConstantB");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
    log::stub("Direct3DDevice9Ex::GetVertexShaderConstantB");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {
    Direct3DVertexBuffer9* vertexBuffer = reinterpret_cast<Direct3DVertexBuffer9*>(pStreamData);
    Com<ID3D11Buffer> buffer;
    vertexBuffer->GetD3D11Buffer(&buffer);
    ID3D11Buffer* bufferPtr = buffer.ptr();
    m_context->IASetVertexBuffers(StreamNumber, 1, &bufferPtr, &Stride, &OffsetInBytes);

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {
    log::stub("Direct3DDevice9Ex::GetStreamSource");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {
    log::stub("Direct3DDevice9Ex::SetStreamSourceFreq");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) {
    log::stub("Direct3DDevice9Ex::GetStreamSourceFreq");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetIndices(IDirect3DIndexBuffer9* pIndexData) {
    log::stub("Direct3DDevice9Ex::SetIndices");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetIndices(IDirect3DIndexBuffer9** ppIndexData) {
    log::stub("Direct3DDevice9Ex::GetIndices");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
    log::stub("Direct3DDevice9Ex::CreatePixelShader");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetPixelShader(IDirect3DPixelShader9* pShader) {
    log::stub("Direct3DDevice9Ex::SetPixelShader");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetPixelShader(IDirect3DPixelShader9** ppShader) {
    log::stub("Direct3DDevice9Ex::GetPixelShader");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
    log::stub("Direct3DDevice9Ex::SetPixelShaderConstantF");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
    log::stub("Direct3DDevice9Ex::GetPixelShaderConstantF");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
    log::stub("Direct3DDevice9Ex::SetPixelShaderConstantI");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
    log::stub("Direct3DDevice9Ex::GetPixelShaderConstantI");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
    log::stub("Direct3DDevice9Ex::SetPixelShaderConstantB");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
    log::stub("Direct3DDevice9Ex::GetPixelShaderConstantB");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* DrawRectPatch) {
    log::stub("Direct3DDevice9Ex::DrawRectPatch");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {
    log::stub("Direct3DDevice9Ex::DrawTriPatch");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::DeletePatch(UINT Handle) {
    log::stub("Direct3DDevice9Ex::DeletePatch");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {
    log::stub("Direct3DDevice9Ex::CreateQuery");
    return D3D_OK;
  }

  // Ex

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetConvolutionMonoKernel(UINT width, UINT height, float* rows, float* columns) {
    log::stub("Direct3DDevice9Ex::SetConvolutionMonoKernel");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::ComposeRects(IDirect3DSurface9* pSrc, IDirect3DSurface9* pDst, IDirect3DVertexBuffer9* pSrcRectDescs, UINT NumRects, IDirect3DVertexBuffer9* pDstRectDescs, D3DCOMPOSERECTSOP Operation, int Xoffset, int Yoffset) {
    log::stub("Direct3DDevice9Ex::ComposeRects");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetGPUThreadPriority(INT* pPriority) {
    if (pPriority == nullptr)
      return D3DERR_INVALIDCALL;

    *pPriority = m_priority;

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetGPUThreadPriority(INT Priority) {
    m_priority = Priority;

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::WaitForVBlank(UINT iSwapChain) {
    log::stub("Direct3DDevice9Ex::WaitForVBlank");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CheckResourceResidency(IDirect3DResource9** pResourceArray, UINT32 NumResources) {
    log::stub("Direct3DDevice9Ex::CheckResourceResidency");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::SetMaximumFrameLatency(UINT MaxLatency) {
    log::stub("Direct3DDevice9Ex::SetMaximumFrameLatency");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetMaximumFrameLatency(UINT* pMaxLatency) {
    log::stub("Direct3DDevice9Ex::GetMaximumFrameLatency");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CheckDeviceState(HWND hDestinationWindow) {
    Direct3DSwapChain9Ex* swapchain = reinterpret_cast<Direct3DSwapChain9Ex*>(m_swapchains[0].ptr());
    return swapchain->TestSwapchain(hDestinationWindow, true);
  }

  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::PresentEx(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags) {
    // Not sure what swapchain to use here, going with this one ~ Josh
    HRESULT result = m_swapchains[0]->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);

    if (m_pendingCursorUpdate.update)
      SetCursorPosition(m_pendingCursorUpdate.x, m_pendingCursorUpdate.y, D3DCURSOR_IMMEDIATE_UPDATE);
    
    return result;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateRenderTargetEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
    log::stub("Direct3DDevice9Ex::CreateRenderTargetEx");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateOffscreenPlainSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
    log::stub("Direct3DDevice9Ex::CreateOffscreenPlainSurfaceEx");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::CreateDepthStencilSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) {
    log::stub("Direct3DDevice9Ex::CreateDepthStencilSurfaceEx");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::ResetEx(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode) {
    log::stub("Direct3DDevice9Ex::ResetEx");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DDevice9Ex::GetDisplayModeEx(UINT iSwapChain, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) {
    log::stub("Direct3DDevice9Ex::GetDisplayModeEx");
    return D3D_OK;
  }

  void Direct3DDevice9Ex::GetParent(Direct3D9Ex** parent) {
    *parent = static_cast<Direct3D9Ex*>( ref(m_parent) );
  }
  void Direct3DDevice9Ex::GetContext(ID3D11DeviceContext** context) {
    *context = ref(m_context);
  }
  void Direct3DDevice9Ex::GetD3D11Device(ID3D11Device** device) {
    *device = ref(m_device);
  }

}