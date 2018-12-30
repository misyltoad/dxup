#include "d3d9_interface.h"
#include "d3d9_device.h"
#include "../util/config.h"
#include <algorithm>
#include <vector>

namespace dxup {

  HRESULT STDMETHODCALLTYPE Direct3D9Ex::QueryInterface(REFIID riid, LPVOID* ppv) {
    InitReturnPtr(ppv);

    if (!ppv)
      return E_POINTER;

    if (riid == __uuidof(IDirect3D9Ex) || riid == __uuidof(IDirect3D9) || riid == __uuidof(IUnknown)) {
      *ppv = ref(this);
      return D3D_OK;
    }

    return E_NOINTERFACE;
  }

  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::RegisterSoftwareDevice(void* pInitializeFunction) {
    log::stub("Direct3D9Ex::RegisterSoftwareDevice");
    return D3D_OK;
  }
  UINT     STDMETHODCALLTYPE Direct3D9Ex::GetAdapterCount() {
    UINT AdapterCount = 0;
    Com<IDXGIAdapter> adapter;
    while (!FAILED(m_dxgiFactory->EnumAdapters(AdapterCount, &adapter)))
      AdapterCount++;

    return AdapterCount;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) {
    if (pIdentifier == nullptr)
      return D3DERR_INVALIDCALL;

    Com<IDXGIAdapter1> adapter;
    HRESULT result = m_dxgiFactory->EnumAdapters1(Adapter, &adapter);
    if (FAILED(result))
      return D3DERR_INVALIDCALL;

    Com<IDXGIOutput> output;
    result = adapter->EnumOutputs(0, &output);
    if (FAILED(result))
      return D3DERR_INVALIDCALL;

    DXGI_ADAPTER_DESC1 desc;
    adapter->GetDesc1(&desc);

    DXGI_OUTPUT_DESC outDesc;
    output->GetDesc(&outDesc);

    strcpy(pIdentifier->Driver, "DXUP Generic Device");
    wcstombs(pIdentifier->Description, desc.Description, MAX_DEVICE_IDENTIFIER_STRING);
    wcstombs(pIdentifier->DeviceName, outDesc.DeviceName, 32);

    std::memset(&pIdentifier->DriverVersion, 0, sizeof(LARGE_INTEGER));

    pIdentifier->VendorId = desc.VendorId;
    pIdentifier->DeviceId = desc.DeviceId;
    pIdentifier->SubSysId = desc.SubSysId;
    pIdentifier->Revision = desc.Revision;
    
    std::memcpy(&pIdentifier->DeviceIdentifier, &desc.AdapterLuid, sizeof(LUID));

    pIdentifier->WHQLLevel = 0;

    return D3D_OK;
  }
  UINT     STDMETHODCALLTYPE Direct3D9Ex::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) {
    if (FAILED(UpdateDisplayModes(Adapter, Format)))
      return 0;

    return (UINT)m_displayModes.size();
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode) {
    return EnumAdapterModes(Adapter, D3DFMT_A8B8G8R8, 0, pMode);
  }

  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed) {
    return D3D_OK;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) {
    return D3D_OK;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) {
    if (pQualityLevels)
      *pQualityLevels = 3;

    return D3D_OK;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) {
    return D3D_OK;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) {
    return D3D_OK;
  }

  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) {
    if (!pCaps)
      return D3DERR_INVALIDCALL;

    // Assuming you're not running a potato if you're using dxup.
    // Feel free to improve.

    pCaps->DeviceType = D3DDEVTYPE_HAL;
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
    pCaps->VertexShaderVersion = D3DVS_VERSION(2, 0); // Only sm1_1 support for now!
    pCaps->MaxVertexShaderConst = 256;
    //pCaps->PixelShaderVersion = D3DPS_VERSION(3, 0);
    pCaps->PixelShaderVersion = D3DPS_VERSION(2, 0);
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
    pCaps->PS20Caps.NumInstructionSlots = 256;
    pCaps->VertexTextureFilterCaps = pCaps->TextureFilterCaps;
    pCaps->MaxVertexShader30InstructionSlots = 32768;
    pCaps->MaxPixelShader30InstructionSlots = 32768;
    pCaps->MaxVShaderInstructionsExecuted = 65535;
    pCaps->MaxPShaderInstructionsExecuted = 65535;

    pCaps->MaxNpatchTessellationLevel = 0.0f;
    pCaps->Reserved5 = 0;

    return D3D_OK;
  }

  HMONITOR STDMETHODCALLTYPE Direct3D9Ex::GetAdapterMonitor(UINT Adapter) {
    Com<IDXGIAdapter1> adapter;
    HRESULT Result = m_dxgiFactory->EnumAdapters1(Adapter, &adapter);
    if (FAILED(Result))
      return NULL;

    Com<IDXGIOutput> output;
    Result = adapter->EnumOutputs(0, &output);
    if (FAILED(Result))
      return NULL;

    DXGI_OUTPUT_DESC outputDesc;
    Result = output->GetDesc(&outputDesc);
    if (FAILED(Result))
      return NULL;

    return outputDesc.Monitor;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
    InitReturnPtr(ppReturnedDeviceInterface);

    if (!pPresentationParameters)
      return D3DERR_INVALIDCALL;

    D3DDISPLAYMODEEX displayMode;
    displayMode.Size = sizeof(D3DDISPLAYMODEEX);
    displayMode.Width = pPresentationParameters->BackBufferWidth;
    displayMode.Height = pPresentationParameters->BackBufferHeight;
    displayMode.RefreshRate = 0;
    displayMode.Format = pPresentationParameters->BackBufferFormat;
    displayMode.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;

    return CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &displayMode, (IDirect3DDevice9Ex**)ppReturnedDeviceInterface);
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) {
    if (!pMode)
      return D3DERR_INVALIDCALL;

    D3DDISPLAYMODEEX exMode;
    HRESULT result = EnumAdapterModeFormatEx(Adapter, Format, NULL, Mode, &exMode);

    pMode->Format = Format;
    pMode->Height = exMode.Height;
    pMode->RefreshRate = exMode.RefreshRate;
    pMode->Width = exMode.Width;

    return result;
  }

  // Direct3D9Ex
  UINT     STDMETHODCALLTYPE Direct3D9Ex::GetAdapterModeCountEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter) {
    log::stub("Direct3D9Ex::GetAdapterModeCountEx");
    return D3D_OK;
  }
  HRESULT  STDMETHODCALLTYPE Direct3D9Ex::EnumAdapterModesEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode) {
    return EnumAdapterModeFormatEx(Adapter, D3DFMT_A8B8G8R8, pFilter, Mode, pMode);
  }
  HRESULT  STDMETHODCALLTYPE Direct3D9Ex::GetAdapterDisplayModeEx(UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) {
    log::stub("Direct3D9Ex::GetAdapterDisplayModeEx");
    return D3D_OK;
  }
  HRESULT  STDMETHODCALLTYPE Direct3D9Ex::CreateDeviceEx(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface) {
    if (!ppReturnedDeviceInterface || !pPresentationParameters)
      return D3DERR_INVALIDCALL;

    InitReturnPtr(ppReturnedDeviceInterface);

    return Direct3DDevice9Ex::Create(
      Adapter,
      hFocusWindow,
      this,
      pPresentationParameters,
      DeviceType,
      true,
      BehaviorFlags,
      ppReturnedDeviceInterface
    );
  }
  HRESULT  STDMETHODCALLTYPE Direct3D9Ex::GetAdapterLUID(UINT Adapter, LUID * pLUID) {
    if (!pLUID)
      return D3DERR_INVALIDCALL;

    Com<IDXGIAdapter1> adapter = nullptr;
    if (!FAILED(m_dxgiFactory->EnumAdapters1(Adapter, &adapter)))
    {
      DXGI_ADAPTER_DESC adapterDesc;      

      if (!FAILED(adapter->GetDesc(&adapterDesc)))
        *pLUID = adapterDesc.AdapterLuid;
      else
        return D3DERR_INVALIDCALL;
    }

    return D3D_OK;
  }

  HRESULT Direct3D9Ex::EnumAdapterModeFormatEx(UINT Adapter, D3DFORMAT Format, const D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode) {
    if (!pMode)
      return D3DERR_INVALIDCALL;

    if (FAILED(UpdateDisplayModes(Adapter, Format)))
      return D3DERR_INVALIDCALL;

    if (m_displayModes.size() <= Mode)
      return D3DERR_INVALIDCALL;

    DXGI_MODE_DESC& RequestedMode = m_displayModes[Mode];
    pMode->Format = Format;
    pMode->Width = RequestedMode.Width;
    pMode->Height = RequestedMode.Height;
    pMode->RefreshRate = RequestedMode.RefreshRate.Denominator;
    pMode->ScanLineOrdering = convert::scanlineOrdering(RequestedMode.ScanlineOrdering);
    pMode->Size = sizeof(D3DDISPLAYMODEEX);

    return D3D_OK;
  }

  HRESULT Direct3D9Ex::UpdateDisplayModes(UINT Adapter, D3DFORMAT Format) {
    if (m_displayModeAdapter == Adapter && m_displayModeFormats == Format && !m_displayModes.empty())
      return D3D_OK;

    m_displayModeAdapter = Adapter;
    m_displayModeFormats = Format;
    m_displayModes.clear();

    Com<IDXGIAdapter1> adapter;
    HRESULT Result = m_dxgiFactory->EnumAdapters1(Adapter, &adapter);
    if (FAILED(Result))
      return D3DERR_INVALIDCALL;

    Com<IDXGIOutput> output;
    Result = adapter->EnumOutputs(0, &output);
    if (FAILED(Result))
      return D3DERR_INVALIDCALL;

    // This is disgusting, what the fuck MS?! ~ Josh
    UINT ModeCount = 0;
    DXGI_FORMAT dxgiFormat = convert::format(Format);
    Result = output->GetDisplayModeList(dxgiFormat, 0, &ModeCount, nullptr);

    m_displayModes.resize(ModeCount);

    Result = output->GetDisplayModeList(dxgiFormat, 0, &ModeCount, m_displayModes.data());
    if (FAILED(Result))
      return D3DERR_INVALIDCALL;

    std::reverse(m_displayModes.begin(), m_displayModes.end());

    return D3D_OK;
  }

  // dxup

  IDXGIFactory1* Direct3D9Ex::GetDXGIFactory() {
    return m_dxgiFactory.ptr();
  }

}