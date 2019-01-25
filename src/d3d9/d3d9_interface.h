#pragma once

#include "d3d9_base.h"

namespace dxup {

  class Direct3D9Ex final : public Unknown<IDirect3D9Ex> {

    public:
      Direct3D9Ex(IDXGIFactory1* dxgiFactory)
        : m_dxgiFactory{ dxgiFactory }
      {}

      HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv) override;

      HRESULT   STDMETHODCALLTYPE RegisterSoftwareDevice(void* pInitializeFunction) override;
      UINT     STDMETHODCALLTYPE GetAdapterCount() override;
      HRESULT   STDMETHODCALLTYPE GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) override;
      UINT     STDMETHODCALLTYPE GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) override;
      HRESULT   STDMETHODCALLTYPE GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode) override;
      HRESULT   STDMETHODCALLTYPE CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed) override;
      HRESULT   STDMETHODCALLTYPE CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) override;
      HRESULT   STDMETHODCALLTYPE CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) override;
      HRESULT   STDMETHODCALLTYPE CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) override;
      HRESULT   STDMETHODCALLTYPE CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) override;
      HRESULT   STDMETHODCALLTYPE GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) override;
      HMONITOR STDMETHODCALLTYPE GetAdapterMonitor(UINT Adapter) override;
      HRESULT   STDMETHODCALLTYPE CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) override;
      HRESULT   STDMETHODCALLTYPE EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) override;

      // Direct3D9Ex
      UINT     STDMETHODCALLTYPE GetAdapterModeCountEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter) override;
      HRESULT  STDMETHODCALLTYPE EnumAdapterModesEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode) override;
      HRESULT  STDMETHODCALLTYPE GetAdapterDisplayModeEx(UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) override;
      HRESULT  STDMETHODCALLTYPE CreateDeviceEx(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface) override;
      HRESULT  STDMETHODCALLTYPE GetAdapterLUID(UINT Adapter, LUID * pLUID) override;

      // dxup

      IDXGIFactory1* GetDXGIFactory();
      HRESULT EnumAdapterModeFormatEx(UINT Adapter, D3DFORMAT Format, const D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode);


      HRESULT UpdateDisplayModes(UINT adapter, D3DFORMAT format);
      void cacheAdapters();
      inline const std::vector<Com<IDXGIAdapter1>>& getAdapterList() {
        return m_adapters;
      }

    private:

      UINT m_displayModeAdapter;
      D3DFORMAT m_displayModeFormats;
      std::vector<DXGI_MODE_DESC> m_displayModes;
      std::vector<Com<IDXGIAdapter1>> m_adapters;
      Com<IDXGIFactory1> m_dxgiFactory;

  };

}