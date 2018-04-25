#include <array>

#include "d3d10_1_device.h"
#include "dxgi_swapchain.h"

extern "C"
{
	using namespace dxup;

	static D3D_FEATURE_LEVEL featureLevel[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	HRESULT STDMETHODCALLTYPE D3D10CreateDevice1(
		_In_  IDXGIAdapter         *pAdapter,
		_In_  D3D10_DRIVER_TYPE    DriverType,
		_In_  HMODULE              Software,
		_In_  UINT                 Flags,
		_In_  D3D10_FEATURE_LEVEL1 HardwareLevel,
		_In_  UINT                 SDKVersion,
		_Out_ ID3D10Device1        **ppDevice
	)
	{
		ID3D11Device* pDX11Device = nullptr;
		ID3D11Device1* pDX11Device1 = nullptr;
		D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_1;

		Flags &= ~D3D10_CREATE_DEVICE_ALLOW_NULL_FROM_MAP;
		Flags &= ~D3D10_CREATE_DEVICE_STRICT_VALIDATION;
		Flags &= ~D3D10_CREATE_DEVICE_DEBUGGABLE;
		Flags |= D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT;
#ifdef _DEBUG
		//Flags |= D3D11_CREATE_DEVICE_DEBUG;
		//Flags |= D3D11_CREATE_DEVICE_DEBUGGABLE;
#endif

		HRESULT result = D3D11CreateDevice(pAdapter, pAdapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, Software, Flags, featureLevel, 6, D3D11_SDK_VERSION, &pDX11Device, &level, nullptr);

		pDX11Device->QueryInterface(__uuidof(ID3D11Device1), (void **)&pDX11Device1);

		*ppDevice = new D3D10Device(pDX11Device1);

		return result;
	}

	HRESULT STDMETHODCALLTYPE D3D10CreateDeviceAndSwapChain1(
		_In_opt_ IDXGIAdapter *pAdapter,
		D3D10_DRIVER_TYPE DriverType,
		HMODULE Software,
		UINT Flags,
		D3D10_FEATURE_LEVEL1 HardwareLevel,
		UINT SDKVersion,
		DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
		IDXGISwapChain **ppSwapChain,
		ID3D10Device1 **ppDevice)
	{
		ID3D11Device* pDX11Device = nullptr;
		ID3D11Device1* pDX11Device1 = nullptr;
		IDXGISwapChain* pSwapChain = nullptr;
		D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_1;

		Flags &= ~D3D10_CREATE_DEVICE_ALLOW_NULL_FROM_MAP;
		Flags &= ~D3D10_CREATE_DEVICE_STRICT_VALIDATION;
		Flags &= ~D3D10_CREATE_DEVICE_DEBUGGABLE;
		Flags |= D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT;
#ifdef _DEBUG
		//Flags |= D3D11_CREATE_DEVICE_DEBUG;
		//Flags |= D3D11_CREATE_DEVICE_DEBUGGABLE;
#endif

		HRESULT result = D3D11CreateDeviceAndSwapChain(pAdapter, pAdapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, Software, Flags, featureLevel, 6, D3D11_SDK_VERSION, pSwapChainDesc, &pSwapChain, &pDX11Device, &level, nullptr);

		pDX11Device->QueryInterface(__uuidof(ID3D11Device1), (void **)&pDX11Device1);

		*ppSwapChain = new DXGISwapChain(pSwapChain);
		*ppDevice = new D3D10Device(pDX11Device1);

		return result;
	}
}