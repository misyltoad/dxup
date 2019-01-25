#include "d3d9_interface.h"
#include "d3d9_device.h"
#include "d3d9_format.h"
#include "../util/config.h"
#include <algorithm>
#include <vector>
#include <cfloat>

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
    cacheAdapters();
    return m_adapters.size();
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) {
    if (pIdentifier == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetAdapterIdentifier: pIdentifier was nullptr.");

    bool fake = config::getBool(config::UseFakes);

    cacheAdapters();

    if (Adapter >= m_adapters.size())
      return log::d3derr(D3DERR_INVALIDCALL, "GetAdapterIdentifier: adapter out of bounds (adapter = %d, range: 0-%d).", Adapter, m_adapters.size());

    if (!fake) {
      IDXGIAdapter1* adapter = m_adapters[Adapter].ptr();

      Com<IDXGIOutput> output;
      HRESULT result = adapter->EnumOutputs(0, &output);
      if (FAILED(result))
        return log::d3derr(D3DERR_INVALIDCALL, "GetAdapterIdentifier: IDXGIAdapter1::EnumOutputs failed.");

      DXGI_ADAPTER_DESC1 desc;
      adapter->GetDesc1(&desc);

      DXGI_OUTPUT_DESC outDesc;
      output->GetDesc(&outDesc);

      convert::wideStringToMultiByte(desc.Description, pIdentifier->Description, MAX_DEVICE_IDENTIFIER_STRING);
      convert::wideStringToMultiByte(outDesc.DeviceName, pIdentifier->DeviceName, 32);
      strcpy(pIdentifier->Driver, "d3d9.dll");
      pIdentifier->DriverVersion.QuadPart = 0;
      pIdentifier->VendorId = desc.VendorId;
      pIdentifier->DeviceId = desc.DeviceId;
      pIdentifier->SubSysId = desc.SubSysId;
      pIdentifier->Revision = desc.Revision;
      std::memset(&pIdentifier->DeviceIdentifier, 0, sizeof(pIdentifier->DeviceIdentifier));
      std::memcpy(&pIdentifier->DeviceIdentifier, &desc.AdapterLuid, sizeof(LUID));
    }
    else {
      strcpy(pIdentifier->Description, config::getString(config::FakeDescription).c_str());
      strcpy(pIdentifier->DeviceName, config::getString(config::FakeDeviceName).c_str());
      strcpy(pIdentifier->Driver, config::getString(config::FakeDriver).c_str());
      pIdentifier->DriverVersion.QuadPart = config::getInt(config::FakeDriverVersion);

      pIdentifier->VendorId = config::getInt(config::FakeVendorId);
      pIdentifier->DeviceId = config::getInt(config::FakeDeviceId);
      pIdentifier->SubSysId = config::getInt(config::FakeSubSysId);
      pIdentifier->Revision = config::getInt(config::FakeRevision);

      // TODO - MINGW - FIXME REIMPLEMENT THIS.
      //UuidFromStringA((RPC_CSTR)config::getString(config::FakeDeviceIdentifier).c_str(), &pIdentifier->DeviceIdentifier);
    }

    pIdentifier->WHQLLevel = 0;

    return D3D_OK;
  }
  UINT     STDMETHODCALLTYPE Direct3D9Ex::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) {
    if (FAILED(UpdateDisplayModes(Adapter, Format)))
      return 0;

    return (UINT)m_displayModes.size();
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode) {
    return EnumAdapterModes(Adapter, D3DFMT_X8R8G8B8, 0, pMode);
  }

  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed) {
    if (Adapter >= GetAdapterCount())
      return log::d3derr(D3DERR_INVALIDCALL, "CheckDeviceType: invalid adapter requested (%d).", Adapter);

    switch (AdapterFormat) {
    case D3DFMT_UNKNOWN:
      if (bWindowed == FALSE)
        return D3DERR_INVALIDCALL;
      else
        return D3DERR_NOTAVAILABLE;
    case D3DFMT_A2R10G10B10:
    case D3DFMT_A2B10G10R10:
    case D3DFMT_X8R8G8B8:
    case D3DFMT_R5G6B5:
      break;
    default:
      return D3DERR_NOTAVAILABLE;
    }

    if (bWindowed != FALSE) {
      switch (BackBufferFormat) {
      case D3DFMT_A2R10G10B10:
      case D3DFMT_A2B10G10R10:
      case D3DFMT_A8R8G8B8:
      case D3DFMT_UNKNOWN:
      case D3DFMT_X8R8G8B8:
      case D3DFMT_R5G6B5:
      case D3DFMT_X1R5G5B5:
      case D3DFMT_A1R5G5B5:
        break;
      default:
        return D3DERR_NOTAVAILABLE;
      }
    }
    else {
      switch (BackBufferFormat) {
      case D3DFMT_UNKNOWN:
        return D3DERR_INVALIDCALL;
      case D3DFMT_A2R10G10B10:
      case D3DFMT_A2B10G10R10:
      case D3DFMT_A8R8G8B8:
      case D3DFMT_X8R8G8B8:
        break;
      default:
        return D3DERR_NOTAVAILABLE;
      }
    }

    return D3D_OK;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) {
    if (Usage & D3DUSAGE_QUERY_SRGBREAD || Usage & D3DUSAGE_QUERY_SRGBWRITE) {
      DXGI_FORMAT possibleTypeless = convert::format(CheckFormat);
      DXGI_FORMAT unorm = convert::makeUntypeless(possibleTypeless, false);

      if (unorm == possibleTypeless)
        return D3DERR_NOTAVAILABLE;
    }

    if (CheckFormat == D3DFMT_INST) // weird hack every driver implements.
      return D3DERR_NOTAVAILABLE; // TODO! Instancing.

    // Table modified from SwiftShader.

    switch (RType) {
    case D3DRTYPE_SURFACE:
      if (Usage & D3DUSAGE_RENDERTARGET) {
        switch (CheckFormat) {
        //case D3DFMT_NULL:			return D3D_OK;
        case D3DFMT_R8G8B8:			return D3D_OK;
        case D3DFMT_R5G6B5:			return D3D_OK;
        case D3DFMT_X1R5G5B5:		return D3D_OK;
        case D3DFMT_A1R5G5B5:		return D3D_OK;
        case D3DFMT_A4R4G4B4:		return D3D_OK;
        case D3DFMT_R3G3B2:			return D3D_OK;
        case D3DFMT_A8R3G3B2:		return D3D_OK;
        case D3DFMT_X4R4G4B4:		return D3D_OK;
        case D3DFMT_A8R8G8B8:		return D3D_OK;
        case D3DFMT_X8R8G8B8:		return D3D_OK;
        case D3DFMT_A8B8G8R8:		return D3D_OK;
        case D3DFMT_X8B8G8R8:		return D3D_OK;
          // Integer HDR formats
        case D3DFMT_G16R16:			return D3D_OK;
        case D3DFMT_A2B10G10R10:	return D3D_OK;
        case D3DFMT_A2R10G10B10:	return D3D_OK;
        case D3DFMT_A16B16G16R16:	return D3D_OK;
          // Floating-point formats
        case D3DFMT_R16F:			return D3D_OK;
        case D3DFMT_G16R16F:		return D3D_OK;
        case D3DFMT_A16B16G16R16F:	return D3D_OK;
        case D3DFMT_R32F:			return D3D_OK;
        case D3DFMT_G32R32F:		return D3D_OK;
        case D3DFMT_A32B32G32R32F:	return D3D_OK;
        default:
          return D3DERR_NOTAVAILABLE;
        }
      }
      else if (Usage & D3DUSAGE_DEPTHSTENCIL) {
        switch (CheckFormat) {
        case D3DFMT_D32:			return D3D_OK;
        case D3DFMT_D24S8:			return D3D_OK;
        case D3DFMT_D24X8:			return D3D_OK;
        case D3DFMT_D16:			return D3D_OK;
        case D3DFMT_D24FS8:			return D3D_OK;
        case D3DFMT_D32F_LOCKABLE:	return D3D_OK;
        case D3DFMT_DF24:			return D3D_OK;
        case D3DFMT_DF16:			return D3D_OK;
        case D3DFMT_INTZ:			return D3D_OK;
        default:
          return D3DERR_NOTAVAILABLE;
        }
      }
      else {
        switch (CheckFormat) {
        case D3DFMT_A8:				return D3D_OK;
        case D3DFMT_R5G6B5:			return D3D_OK;
        case D3DFMT_X1R5G5B5:		return D3D_OK;
        case D3DFMT_A1R5G5B5:		return D3D_OK;
        case D3DFMT_A4R4G4B4:		return D3D_OK;
        case D3DFMT_R3G3B2:			return D3D_OK;
        case D3DFMT_A8R3G3B2:		return D3D_OK;
        case D3DFMT_X4R4G4B4:		return D3D_OK;
        case D3DFMT_R8G8B8:			return D3D_OK;
        case D3DFMT_X8R8G8B8:		return D3D_OK;
        case D3DFMT_A8R8G8B8:		return D3D_OK;
        case D3DFMT_X8B8G8R8:		return D3D_OK;
        case D3DFMT_A8B8G8R8:		return D3D_OK;
          // Paletted formats
        case D3DFMT_P8:				return D3D_OK;
        case D3DFMT_A8P8:			return D3D_OK;
          // Integer HDR formats
        case D3DFMT_G16R16:			return D3D_OK;
        case D3DFMT_A2R10G10B10:	return D3D_OK;
        case D3DFMT_A2B10G10R10:	return D3D_OK;
        case D3DFMT_A16B16G16R16:	return D3D_OK;
          // Compressed formats
        case D3DFMT_DXT1:			return D3D_OK;
        case D3DFMT_DXT2:			return D3D_OK;
        case D3DFMT_DXT3:			return D3D_OK;
        case D3DFMT_DXT4:			return D3D_OK;
        case D3DFMT_DXT5:			return D3D_OK;
        case D3DFMT_ATI1:			return D3D_OK;
        case D3DFMT_ATI2:			return D3D_OK;
          // Floating-point formats
        case D3DFMT_R16F:			return D3D_OK;
        case D3DFMT_G16R16F:		return D3D_OK;
        case D3DFMT_A16B16G16R16F:	return D3D_OK;
        case D3DFMT_R32F:			return D3D_OK;
        case D3DFMT_G32R32F:		return D3D_OK;
        case D3DFMT_A32B32G32R32F:	return D3D_OK;
          // Bump map formats
        case D3DFMT_V8U8:			return D3D_OK;
        case D3DFMT_L6V5U5:			return D3D_OK;
        case D3DFMT_X8L8V8U8:		return D3D_OK;
        case D3DFMT_Q8W8V8U8:		return D3D_OK;
        case D3DFMT_V16U16:			return D3D_OK;
        case D3DFMT_A2W10V10U10:	return D3D_OK;
        case D3DFMT_Q16W16V16U16:	return D3D_OK;
          // Luminance formats
        case D3DFMT_L8:				return D3D_OK;
        case D3DFMT_A4L4:			return D3D_OK;
        case D3DFMT_L16:			return D3D_OK;
        case D3DFMT_A8L8:			return D3D_OK;
          // Depth Bounds Test
        //case D3DFMT_NVDB:			return D3D_OK;
          // Transparency anti-aliasing
        //case D3DFMT_ATOC:			return D3D_OK;
        default:
          return D3DERR_NOTAVAILABLE;
        }
      }
    case D3DRTYPE_VOLUME:
      switch (CheckFormat) {
      case D3DFMT_A8:					return D3D_OK;
      case D3DFMT_R5G6B5:				return D3D_OK;
      case D3DFMT_X1R5G5B5:			return D3D_OK;
      case D3DFMT_A1R5G5B5:			return D3D_OK;
      case D3DFMT_A4R4G4B4:			return D3D_OK;
      case D3DFMT_R3G3B2:				return D3D_OK;
      case D3DFMT_A8R3G3B2:			return D3D_OK;
      case D3DFMT_X4R4G4B4:			return D3D_OK;
      case D3DFMT_R8G8B8:				return D3D_OK;
      case D3DFMT_X8R8G8B8:			return D3D_OK;
      case D3DFMT_A8R8G8B8:			return D3D_OK;
      case D3DFMT_X8B8G8R8:			return D3D_OK;
      case D3DFMT_A8B8G8R8:			return D3D_OK;
        // Paletted formats
      case D3DFMT_P8:					return D3D_OK;
      case D3DFMT_A8P8:				return D3D_OK;
        // Integer HDR formats
      case D3DFMT_G16R16:				return D3D_OK;
      case D3DFMT_A2R10G10B10:		return D3D_OK;
      case D3DFMT_A2B10G10R10:		return D3D_OK;
      case D3DFMT_A16B16G16R16:		return D3D_OK;
        // Compressed formats
      case D3DFMT_DXT1:				return D3D_OK;
      case D3DFMT_DXT2:				return D3D_OK;
      case D3DFMT_DXT3:				return D3D_OK;
      case D3DFMT_DXT4:				return D3D_OK;
      case D3DFMT_DXT5:				return D3D_OK;
      case D3DFMT_ATI1:				return D3D_OK;
      case D3DFMT_ATI2:				return D3D_OK;
        // Floating-point formats
      case D3DFMT_R16F:				return D3D_OK;
      case D3DFMT_G16R16F:			return D3D_OK;
      case D3DFMT_A16B16G16R16F:		return D3D_OK;
      case D3DFMT_R32F:				return D3D_OK;
      case D3DFMT_G32R32F:			return D3D_OK;
      case D3DFMT_A32B32G32R32F:		return D3D_OK;
        // Bump map formats
      case D3DFMT_V8U8:				return D3D_OK;
      case D3DFMT_L6V5U5:				return D3D_OK;
      case D3DFMT_X8L8V8U8:			return D3D_OK;
      case D3DFMT_Q8W8V8U8:			return D3D_OK;
      case D3DFMT_V16U16:				return D3D_OK;
      case D3DFMT_A2W10V10U10:		return D3D_OK;
      case D3DFMT_Q16W16V16U16:		return D3D_OK;
        // Luminance formats
      case D3DFMT_L8:					return D3D_OK;
      case D3DFMT_A4L4:				return D3D_OK;
      case D3DFMT_L16:				return D3D_OK;
      case D3DFMT_A8L8:				return D3D_OK;
      default:
        return D3DERR_NOTAVAILABLE;
      }
    case D3DRTYPE_CUBETEXTURE:
      if (Usage & D3DUSAGE_RENDERTARGET) {
        switch (CheckFormat) {
        //case D3DFMT_NULL:			return D3D_OK;
        case D3DFMT_R8G8B8:			return D3D_OK;
        case D3DFMT_R5G6B5:			return D3D_OK;
        case D3DFMT_X1R5G5B5:		return D3D_OK;
        case D3DFMT_A1R5G5B5:		return D3D_OK;
        case D3DFMT_A4R4G4B4:		return D3D_OK;
        case D3DFMT_R3G3B2:			return D3D_OK;
        case D3DFMT_A8R3G3B2:		return D3D_OK;
        case D3DFMT_X4R4G4B4:		return D3D_OK;
        case D3DFMT_A8R8G8B8:		return D3D_OK;
        case D3DFMT_X8R8G8B8:		return D3D_OK;
        case D3DFMT_A8B8G8R8:		return D3D_OK;
        case D3DFMT_X8B8G8R8:		return D3D_OK;
          // Integer HDR formats
        case D3DFMT_G16R16:			return D3D_OK;
        case D3DFMT_A2B10G10R10:	return D3D_OK;
        case D3DFMT_A2R10G10B10:	return D3D_OK;
        case D3DFMT_A16B16G16R16:	return D3D_OK;
          // Floating-point formats
        case D3DFMT_R16F:			return D3D_OK;
        case D3DFMT_G16R16F:		return D3D_OK;
        case D3DFMT_A16B16G16R16F:	return D3D_OK;
        case D3DFMT_R32F:			return D3D_OK;
        case D3DFMT_G32R32F:		return D3D_OK;
        case D3DFMT_A32B32G32R32F:	return D3D_OK;
        default:
          return D3DERR_NOTAVAILABLE;
        }
      }
      else if (Usage & D3DUSAGE_DEPTHSTENCIL) {
        switch (CheckFormat) {
        case D3DFMT_D32:			return D3D_OK;
        case D3DFMT_D24S8:			return D3D_OK;
        case D3DFMT_D24X8:			return D3D_OK;
        case D3DFMT_D16:			return D3D_OK;
        case D3DFMT_D24FS8:			return D3D_OK;
        case D3DFMT_D32F_LOCKABLE:	return D3D_OK;
        case D3DFMT_DF24:			return D3D_OK;
        case D3DFMT_DF16:			return D3D_OK;
        case D3DFMT_INTZ:			return D3D_OK;
        default:
          return D3DERR_NOTAVAILABLE;
        }
      }
      else {
        switch (CheckFormat) {
        case D3DFMT_A8:				return D3D_OK;
        case D3DFMT_R5G6B5:			return D3D_OK;
        case D3DFMT_X1R5G5B5:		return D3D_OK;
        case D3DFMT_A1R5G5B5:		return D3D_OK;
        case D3DFMT_A4R4G4B4:		return D3D_OK;
        case D3DFMT_R3G3B2:			return D3D_OK;
        case D3DFMT_A8R3G3B2:		return D3D_OK;
        case D3DFMT_X4R4G4B4:		return D3D_OK;
        case D3DFMT_R8G8B8:			return D3D_OK;
        case D3DFMT_X8R8G8B8:		return D3D_OK;
        case D3DFMT_A8R8G8B8:		return D3D_OK;
        case D3DFMT_X8B8G8R8:		return D3D_OK;
        case D3DFMT_A8B8G8R8:		return D3D_OK;
          // Paletted formats
        case D3DFMT_P8:				return D3D_OK;
        case D3DFMT_A8P8:			return D3D_OK;
          // Integer HDR formats
        case D3DFMT_G16R16:			return D3D_OK;
        case D3DFMT_A2R10G10B10:	return D3D_OK;
        case D3DFMT_A2B10G10R10:	return D3D_OK;
        case D3DFMT_A16B16G16R16:	return D3D_OK;
          // Compressed formats
        case D3DFMT_DXT1:			return D3D_OK;
        case D3DFMT_DXT2:			return D3D_OK;
        case D3DFMT_DXT3:			return D3D_OK;
        case D3DFMT_DXT4:			return D3D_OK;
        case D3DFMT_DXT5:			return D3D_OK;
        case D3DFMT_ATI1:			return D3D_OK;
        case D3DFMT_ATI2:			return D3D_OK;
          // Floating-point formats
        case D3DFMT_R16F:			return D3D_OK;
        case D3DFMT_G16R16F:		return D3D_OK;
        case D3DFMT_A16B16G16R16F:	return D3D_OK;
        case D3DFMT_R32F:			return D3D_OK;
        case D3DFMT_G32R32F:		return D3D_OK;
        case D3DFMT_A32B32G32R32F:	return D3D_OK;
          // Bump map formats
        case D3DFMT_V8U8:			return D3D_OK;
        case D3DFMT_L6V5U5:			return D3D_OK;
        case D3DFMT_X8L8V8U8:		return D3D_OK;
        case D3DFMT_Q8W8V8U8:		return D3D_OK;
        case D3DFMT_V16U16:			return D3D_OK;
        case D3DFMT_A2W10V10U10:	return D3D_OK;
        case D3DFMT_Q16W16V16U16:	return D3D_OK;
          // Luminance formats
        case D3DFMT_L8:				return D3D_OK;
        case D3DFMT_A4L4:			return D3D_OK;
        case D3DFMT_L16:			return D3D_OK;
        case D3DFMT_A8L8:			return D3D_OK;
        default:
          return D3DERR_NOTAVAILABLE;
        }
      }
    case D3DRTYPE_VOLUMETEXTURE:
      switch (CheckFormat) {
      case D3DFMT_A8:					return D3D_OK;
      case D3DFMT_R5G6B5:				return D3D_OK;
      case D3DFMT_X1R5G5B5:			return D3D_OK;
      case D3DFMT_A1R5G5B5:			return D3D_OK;
      case D3DFMT_A4R4G4B4:			return D3D_OK;
      case D3DFMT_R3G3B2:				return D3D_OK;
      case D3DFMT_A8R3G3B2:			return D3D_OK;
      case D3DFMT_X4R4G4B4:			return D3D_OK;
      case D3DFMT_R8G8B8:				return D3D_OK;
      case D3DFMT_X8R8G8B8:			return D3D_OK;
      case D3DFMT_A8R8G8B8:			return D3D_OK;
      case D3DFMT_X8B8G8R8:			return D3D_OK;
      case D3DFMT_A8B8G8R8:			return D3D_OK;
        // Paletted formats
      case D3DFMT_P8:					return D3D_OK;
      case D3DFMT_A8P8:				return D3D_OK;
        // Integer HDR formats
      case D3DFMT_G16R16:				return D3D_OK;
      case D3DFMT_A2R10G10B10:		return D3D_OK;
      case D3DFMT_A2B10G10R10:		return D3D_OK;
      case D3DFMT_A16B16G16R16:		return D3D_OK;
        // Compressed formats
      case D3DFMT_DXT1:				return D3D_OK;
      case D3DFMT_DXT2:				return D3D_OK;
      case D3DFMT_DXT3:				return D3D_OK;
      case D3DFMT_DXT4:				return D3D_OK;
      case D3DFMT_DXT5:				return D3D_OK;
      case D3DFMT_ATI1:				return D3D_OK;
      case D3DFMT_ATI2:				return D3D_OK;
        // Floating-point formats
      case D3DFMT_R16F:				return D3D_OK;
      case D3DFMT_G16R16F:			return D3D_OK;
      case D3DFMT_A16B16G16R16F:		return D3D_OK;
      case D3DFMT_R32F:				return D3D_OK;
      case D3DFMT_G32R32F:			return D3D_OK;
      case D3DFMT_A32B32G32R32F:		return D3D_OK;
        // Bump map formats
      case D3DFMT_V8U8:				return D3D_OK;
      case D3DFMT_L6V5U5:				return D3D_OK;
      case D3DFMT_X8L8V8U8:			return D3D_OK;
      case D3DFMT_Q8W8V8U8:			return D3D_OK;
      case D3DFMT_V16U16:				return D3D_OK;
      case D3DFMT_A2W10V10U10:		return D3D_OK;
      case D3DFMT_Q16W16V16U16:		return D3D_OK;
        // Luminance formats
      case D3DFMT_L8:					return D3D_OK;
      case D3DFMT_A4L4:				return D3D_OK;
      case D3DFMT_L16:				return D3D_OK;
      case D3DFMT_A8L8:				return D3D_OK;
      default:
        return D3DERR_NOTAVAILABLE;
      }
    case D3DRTYPE_TEXTURE:
      if (Usage & D3DUSAGE_RENDERTARGET) {
        switch (CheckFormat) {
        //case D3DFMT_NULL:			return D3D_OK;
        case D3DFMT_R8G8B8:			return D3D_OK;
        case D3DFMT_R5G6B5:			return D3D_OK;
        case D3DFMT_X1R5G5B5:		return D3D_OK;
        case D3DFMT_A1R5G5B5:		return D3D_OK;
        case D3DFMT_A4R4G4B4:		return D3D_OK;
        case D3DFMT_R3G3B2:			return D3D_OK;
        case D3DFMT_A8R3G3B2:		return D3D_OK;
        case D3DFMT_X4R4G4B4:		return D3D_OK;
        case D3DFMT_A8R8G8B8:		return D3D_OK;
        case D3DFMT_X8R8G8B8:		return D3D_OK;
        case D3DFMT_A8B8G8R8:		return D3D_OK;
        case D3DFMT_X8B8G8R8:		return D3D_OK;
          // Integer HDR formats
        case D3DFMT_G16R16:			return D3D_OK;
        case D3DFMT_A2B10G10R10:	return D3D_OK;
        case D3DFMT_A2R10G10B10:	return D3D_OK;
        case D3DFMT_A16B16G16R16:	return D3D_OK;
          // Floating-point formats
        case D3DFMT_R16F:			return D3D_OK;
        case D3DFMT_G16R16F:		return D3D_OK;
        case D3DFMT_A16B16G16R16F:	return D3D_OK;
        case D3DFMT_R32F:			return D3D_OK;
        case D3DFMT_G32R32F:		return D3D_OK;
        case D3DFMT_A32B32G32R32F:	return D3D_OK;
        default:
          return D3DERR_NOTAVAILABLE;
        }
      }
      else if (Usage & D3DUSAGE_DEPTHSTENCIL) {
        switch (CheckFormat) {
        case D3DFMT_D32:			return D3D_OK;
        case D3DFMT_D24S8:			return D3D_OK;
        case D3DFMT_D24X8:			return D3D_OK;
        case D3DFMT_D16:			return D3D_OK;
        case D3DFMT_D24FS8:			return D3D_OK;
        case D3DFMT_D32F_LOCKABLE:	return D3D_OK;
        case D3DFMT_DF24:			return D3D_OK;
        case D3DFMT_DF16:			return D3D_OK;
        case D3DFMT_INTZ:			return D3D_OK;
        default:
          return D3DERR_NOTAVAILABLE;
        }
      }
      else {
        switch (CheckFormat) {
        //case D3DFMT_NULL:			return D3D_OK;
        case D3DFMT_A8:				return D3D_OK;
        case D3DFMT_R5G6B5:			return D3D_OK;
        case D3DFMT_X1R5G5B5:		return D3D_OK;
        case D3DFMT_A1R5G5B5:		return D3D_OK;
        case D3DFMT_A4R4G4B4:		return D3D_OK;
        case D3DFMT_R3G3B2:			return D3D_OK;
        case D3DFMT_A8R3G3B2:		return D3D_OK;
        case D3DFMT_X4R4G4B4:		return D3D_OK;
        case D3DFMT_R8G8B8:			return D3D_OK;
        case D3DFMT_X8R8G8B8:		return D3D_OK;
        case D3DFMT_A8R8G8B8:		return D3D_OK;
        case D3DFMT_X8B8G8R8:		return D3D_OK;
        case D3DFMT_A8B8G8R8:		return D3D_OK;
          // Paletted formats
        case D3DFMT_P8:				return D3D_OK;
        case D3DFMT_A8P8:			return D3D_OK;
          // Integer HDR formats
        case D3DFMT_G16R16:			return D3D_OK;
        case D3DFMT_A2R10G10B10:	return D3D_OK;
        case D3DFMT_A2B10G10R10:	return D3D_OK;
        case D3DFMT_A16B16G16R16:	return D3D_OK;
          // Compressed formats
        case D3DFMT_DXT1:			return D3D_OK;
        case D3DFMT_DXT2:			return D3D_OK;
        case D3DFMT_DXT3:			return D3D_OK;
        case D3DFMT_DXT4:			return D3D_OK;
        case D3DFMT_DXT5:			return D3D_OK;
        case D3DFMT_ATI1:			return D3D_OK;
        case D3DFMT_ATI2:			return D3D_OK;
          // Floating-point formats
        case D3DFMT_R16F:			return D3D_OK;
        case D3DFMT_G16R16F:		return D3D_OK;
        case D3DFMT_A16B16G16R16F:	return D3D_OK;
        case D3DFMT_R32F:			return D3D_OK;
        case D3DFMT_G32R32F:		return D3D_OK;
        case D3DFMT_A32B32G32R32F:	return D3D_OK;
          // Bump map formats
        case D3DFMT_V8U8:			return D3D_OK;
        case D3DFMT_L6V5U5:			return D3D_OK;
        case D3DFMT_X8L8V8U8:		return D3D_OK;
        case D3DFMT_Q8W8V8U8:		return D3D_OK;
        case D3DFMT_V16U16:			return D3D_OK;
        case D3DFMT_A2W10V10U10:	return D3D_OK;
        case D3DFMT_Q16W16V16U16:	return D3D_OK;
          // Luminance formats
        case D3DFMT_L8:				return D3D_OK;
        case D3DFMT_A4L4:			return D3D_OK;
        case D3DFMT_L16:			return D3D_OK;
        case D3DFMT_A8L8:			return D3D_OK;
          // Depth formats
        case D3DFMT_D32:			return D3D_OK;
        case D3DFMT_D24S8:			return D3D_OK;
        case D3DFMT_D24X8:			return D3D_OK;
        case D3DFMT_D16:			return D3D_OK;
        case D3DFMT_D24FS8:			return D3D_OK;
        case D3DFMT_D32F_LOCKABLE:	return D3D_OK;
        case D3DFMT_DF24:			return D3D_OK;
        case D3DFMT_DF16:			return D3D_OK;
        case D3DFMT_INTZ:			return D3D_OK;
        default:
          return D3DERR_NOTAVAILABLE;
        }
      }
    case D3DRTYPE_VERTEXBUFFER:
      if (CheckFormat == D3DFMT_VERTEXDATA)
        return D3D_OK;
      else
        return D3DERR_NOTAVAILABLE;
    case D3DRTYPE_INDEXBUFFER:
      switch (CheckFormat) {
      case D3DFMT_INDEX16:
      case D3DFMT_INDEX32:
        return D3D_OK;
      default:
        return D3DERR_NOTAVAILABLE;
      };
    default:
      return D3DERR_NOTAVAILABLE;
    }
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) {
    if (Adapter >= GetAdapterCount())
      return log::d3derr(D3DERR_INVALIDCALL, "CheckDeviceMultiSampleType: invalid adapter requested (%d).", Adapter);

    if (pQualityLevels) {
      if (MultiSampleType == D3DMULTISAMPLE_NONMASKABLE)
        *pQualityLevels = 4;
      else
        *pQualityLevels = 1;
    }

    if (MultiSampleType == D3DMULTISAMPLE_NONE ||
      MultiSampleType == D3DMULTISAMPLE_NONMASKABLE ||
      MultiSampleType == D3DMULTISAMPLE_2_SAMPLES ||
      MultiSampleType == D3DMULTISAMPLE_4_SAMPLES ||
      MultiSampleType == D3DMULTISAMPLE_8_SAMPLES ||
      MultiSampleType == D3DMULTISAMPLE_16_SAMPLES) {
      if (CheckDeviceFormat(Adapter, DeviceType, D3DFMT_X8R8G8B8, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, SurfaceFormat) == D3D_OK ||
        CheckDeviceFormat(Adapter, DeviceType, D3DFMT_X8R8G8B8, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, SurfaceFormat) == D3D_OK) {
        if (SurfaceFormat != D3DFMT_D32F_LOCKABLE && SurfaceFormat != D3DFMT_D16_LOCKABLE)
          return D3D_OK;
      }
    }

    return D3DERR_NOTAVAILABLE;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) {
    return D3D_OK;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) {
    return D3D_OK;
  }

  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) {
    if (!pCaps)
      return log::d3derr(D3DERR_INVALIDCALL, "GetDeviceCaps: pCaps was nullptr.");

    std::string shaderModel = config::getString(config::ShaderModel);

    // NOTE: Based on my setup on D3D9 native. - Josh
    pCaps->DeviceType = D3DDEVTYPE_HAL;
    pCaps->AdapterOrdinal = Adapter;
    pCaps->Caps = 131072;
    pCaps->Caps2 = 3758227456;
    pCaps->Caps3 = 928;
    pCaps->PresentationIntervals = 2147483663;
    pCaps->CursorCaps = 1;
    pCaps->DevCaps = 1818352;
    pCaps->PrimitiveMiscCaps = 4181234;
    pCaps->RasterCaps = 259219857;
    pCaps->ZCmpCaps = 255;
    pCaps->SrcBlendCaps = 16383;
    pCaps->DestBlendCaps = 16383;
    pCaps->AlphaCmpCaps = 255;
    pCaps->ShadeCaps = 541192;
    pCaps->TextureCaps = 126021;
    pCaps->TextureFilterCaps = 117638912;
    pCaps->CubeTextureFilterCaps = 50529024;
    pCaps->VolumeTextureFilterCaps = 117638912;
    pCaps->TextureAddressCaps = 63;
    pCaps->VolumeTextureAddressCaps = 63;
    pCaps->LineCaps = 31;
    pCaps->MaxTextureWidth = 16384;
    pCaps->MaxTextureHeight = 16384;
    pCaps->MaxVolumeExtent = 8192;
    pCaps->MaxTextureRepeat = 8192;
    pCaps->MaxTextureAspectRatio = 8192;
    pCaps->MaxAnisotropy = 16;
    pCaps->MaxVertexW = 1e10f;
    pCaps->GuardBandLeft = -32768.0f;
    pCaps->GuardBandTop = -32768.0f;
    pCaps->GuardBandRight = 32768.0f;
    pCaps->GuardBandBottom = 32768.0f;
    pCaps->ExtentsAdjust = 0.0f;
    pCaps->StencilCaps = 511;
    pCaps->FVFCaps = 1048584;
    pCaps->TextureOpCaps = 67108863;
    pCaps->MaxTextureBlendStages = 8;

    pCaps->MaxSimultaneousTextures = 8;

    pCaps->VertexProcessingCaps = 379;
    pCaps->MaxActiveLights = 8;
    pCaps->MaxUserClipPlanes = 6;
    pCaps->MaxVertexBlendMatrices = 4;
    pCaps->MaxVertexBlendMatrixIndex = 0;
    pCaps->MaxPointSize = 256.0f;
    pCaps->MaxPrimitiveCount = 5592405;
    pCaps->MaxVertexIndex = 16777215;
    pCaps->MaxStreams = 16;
    pCaps->MaxStreamStride = 508;

    if (shaderModel == "3") {
      pCaps->VertexShaderVersion = D3DVS_VERSION(3, 0);
      pCaps->PixelShaderVersion = D3DPS_VERSION(3, 0);
    }
    else if (shaderModel == "2b" || shaderModel == "2B" || shaderModel == "2") {
      pCaps->VertexShaderVersion = D3DVS_VERSION(2, 0);
      pCaps->PixelShaderVersion = D3DPS_VERSION(2, 0);
    }
    else if (shaderModel == "1") {
      pCaps->VertexShaderVersion = D3DVS_VERSION(1, 4);
      pCaps->PixelShaderVersion = D3DPS_VERSION(1, 4);
    }
    pCaps->MaxVertexShaderConst = 256;
    pCaps->PixelShader1xMaxValue = FLT_MAX;
    pCaps->DevCaps2 = 113;
    pCaps->MaxNpatchTessellationLevel = 1.0f;
    pCaps->Reserved5 = 0;
    pCaps->MasterAdapterOrdinal = 0;
    pCaps->AdapterOrdinalInGroup = 0;
    pCaps->NumberOfAdaptersInGroup = 2;
    pCaps->DeclTypes = 1023;
    pCaps->NumSimultaneousRTs = 4;
    pCaps->StretchRectFilterCaps = 50332416;

    pCaps->VS20Caps.Caps = 1;
    pCaps->VS20Caps.DynamicFlowControlDepth = 24;
    pCaps->VS20Caps.NumTemps = 32;
    pCaps->VS20Caps.StaticFlowControlDepth = 4;

    pCaps->PS20Caps.Caps = 31;
    pCaps->PS20Caps.DynamicFlowControlDepth = 24;
    pCaps->PS20Caps.NumTemps = 32;
    pCaps->PS20Caps.StaticFlowControlDepth = 4;

    if (shaderModel == "3" || shaderModel == "2b" || shaderModel == "2B")
      pCaps->PS20Caps.NumInstructionSlots = 512;
    else
      pCaps->PS20Caps.NumInstructionSlots = 256;

    pCaps->VertexTextureFilterCaps = 50332416;
    pCaps->MaxVShaderInstructionsExecuted = 4294967295;
    pCaps->MaxPShaderInstructionsExecuted = 4294967295;

    if (shaderModel == "3") {
      pCaps->MaxVertexShader30InstructionSlots = 32768;
      pCaps->MaxPixelShader30InstructionSlots = 32768;
    }
    else {
      pCaps->MaxVertexShader30InstructionSlots = 0;
      pCaps->MaxPixelShader30InstructionSlots = 0;
    }

    return D3D_OK;
  }

  HMONITOR STDMETHODCALLTYPE Direct3D9Ex::GetAdapterMonitor(UINT Adapter) {
    cacheAdapters();

    if (Adapter >= m_adapters.size()) {
      log::warn("GetAdapterMonitor: adapter out of bounds (adapter = %d, range: 0-%d). Returning null montor.", Adapter, m_adapters.size());
      return nullptr;
    }

    IDXGIAdapter1* adapter = m_adapters[Adapter].ptr();

    Com<IDXGIOutput> output;
    HRESULT result = adapter->EnumOutputs(0, &output);
    if (FAILED(result))
      return NULL;

    DXGI_OUTPUT_DESC outputDesc;
    result = output->GetDesc(&outputDesc);
    if (FAILED(result))
      return NULL;

    return outputDesc.Monitor;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
    InitReturnPtr(ppReturnedDeviceInterface);

    if (!pPresentationParameters)
      return log::d3derr(D3DERR_INVALIDCALL, "CreateDevice: pPresentationParameters was null.");

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
    if (pMode == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "EnumAdapterModes: pMode was nullptr.");

    D3DDISPLAYMODEEX exMode;
    HRESULT result = EnumAdapterModeFormatEx(Adapter, Format, NULL, Mode, &exMode);
    if (FAILED(result))
      return result;

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
    if (ppReturnedDeviceInterface == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "CreateDeviceEx: ppReturnedDeviceInterface was nullptr");

    if (pPresentationParameters == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "CreateDeviceEx: pPresentationParameters was nullptr");

    InitReturnPtr(ppReturnedDeviceInterface);

    cacheAdapters();
    log::msg("Creating D3D9 device with adapter: %d", Adapter);

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
      return log::d3derr(D3DERR_INVALIDCALL, "GetAdapterLUID: pLUID was nullptr");

    cacheAdapters();

    if (Adapter >= m_adapters.size())
      return log::d3derr(D3DERR_INVALIDCALL, "GetAdapterLUID: adapter out of bounds (adapter = %d, range: 0-%d).", Adapter, m_adapters.size());

    IDXGIAdapter1* adapter = m_adapters[Adapter].ptr();

    DXGI_ADAPTER_DESC adapterDesc;
    adapter->GetDesc(&adapterDesc);

    *pLUID = adapterDesc.AdapterLuid;

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
    pMode->RefreshRate = RequestedMode.RefreshRate.Numerator / RequestedMode.RefreshRate.Denominator;
    pMode->ScanLineOrdering = convert::scanlineOrdering(RequestedMode.ScanlineOrdering);
    pMode->Size = sizeof(D3DDISPLAYMODEEX);

    return D3D_OK;
  }

  HRESULT Direct3D9Ex::UpdateDisplayModes(UINT Adapter, D3DFORMAT Format) {
    if (m_displayModeAdapter == Adapter && m_displayModeFormats == Format && !m_displayModes.empty())
      return D3D_OK;

    cacheAdapters();

    m_displayModeAdapter = Adapter;
    m_displayModeFormats = Format;
    m_displayModes.clear();

    IDXGIAdapter1* adapter = m_adapters[Adapter].ptr();

    Com<IDXGIOutput> output;
    HRESULT result = adapter->EnumOutputs(0, &output);
    if (FAILED(result))
      return D3DERR_INVALIDCALL;

    // This is disgusting, what the fuck MS?! ~ Josh
    UINT ModeCount = 0;

    DXGI_FORMAT dxgiFormat = convert::format(Format);
    dxgiFormat = convert::makeSwapchainCompliant(dxgiFormat);

    result = output->GetDisplayModeList(dxgiFormat, 0, &ModeCount, nullptr);

    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "UpdateDisplayModes: GetDisplayModeList failed to get mode count.");

    m_displayModes.resize(ModeCount);

    result = output->GetDisplayModeList(dxgiFormat, 0, &ModeCount, m_displayModes.data());
    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "UpdateDisplayModes: GetDisplayModeList failed to list modes.");

    std::reverse(m_displayModes.begin(), m_displayModes.end());

    return D3D_OK;
  }

  void Direct3D9Ex::cacheAdapters() {
    if (!m_adapters.empty())
      return;

    UINT i = 0;
    Com<IDXGIAdapter1> adapter;

    while (!FAILED(m_dxgiFactory->EnumAdapters1(i, &adapter))) {
      Com<IDXGIOutput> output;

      // Only return adapters with outputs.
      if (!FAILED(adapter->EnumOutputs(0, &output)))
        m_adapters.emplace_back(adapter.ptr());

      adapter = nullptr;

      i++;
    }
  }

  IDXGIFactory1* Direct3D9Ex::GetDXGIFactory() {
    return m_dxgiFactory.ptr();
  }

}