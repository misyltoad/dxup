#include "dxgi_factory.h"
#include "dxgi_swapchain.h"
#include "../d3d10_1/d3d10_1_device.h"

namespace dxup
{
	DXGIFactory::DXGIFactory(IDXGIFactory1* base)
	{
		m_base = base;
	}


	HRESULT STDMETHODCALLTYPE DXGIFactory::QueryInterface(REFIID riid, void** ppvObject) {
		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)
			|| riid == __uuidof(IDXGIObject)
			|| riid == __uuidof(IDXGIFactory)
			|| riid == __uuidof(IDXGIFactory1)
			|| riid == __uuidof(IDXGIFactory2)) {
			*ppvObject = this;
			return S_OK;
		}

		return m_base->QueryInterface(riid, ppvObject);
	}


	HRESULT STDMETHODCALLTYPE DXGIFactory::GetParent(REFIID riid, void** ppParent)
	{
		return m_base->GetParent(riid, ppParent);
	}


	HRESULT STDMETHODCALLTYPE DXGIFactory::CreateSoftwareAdapter(
		HMODULE         Module,
		IDXGIAdapter**  ppAdapter)
	{
		return m_base->CreateSoftwareAdapter(Module, ppAdapter);
	}


	HRESULT STDMETHODCALLTYPE DXGIFactory::CreateSwapChain(
		IUnknown*             pDevice,
		DXGI_SWAP_CHAIN_DESC* pDesc,
		IDXGISwapChain**      ppSwapChain)
	{
		IDXGISwapChain* originalSwapchain = nullptr;

		auto* dxupDevice = static_cast<D3D10Device*>(pDevice);

		HRESULT result = m_base->CreateSwapChain(dxupDevice->GetD3D11Interface(), pDesc, &originalSwapchain);

		if (originalSwapchain)
		{
			DXGISwapChain* ourSwapchain = new DXGISwapChain(originalSwapchain);

			if (ppSwapChain)
				*ppSwapChain = ourSwapchain;
		}

		return result;
	}


	HRESULT STDMETHODCALLTYPE DXGIFactory::EnumAdapters(
		UINT            Adapter,
		IDXGIAdapter**  ppAdapter)
	{
		return m_base->EnumAdapters(Adapter, ppAdapter);
	}


	HRESULT STDMETHODCALLTYPE DXGIFactory::EnumAdapters1(
		UINT            Adapter,
		IDXGIAdapter1** ppAdapter)
	{
		return m_base->EnumAdapters1(Adapter, ppAdapter);
	}


	HRESULT STDMETHODCALLTYPE DXGIFactory::GetWindowAssociation(HWND *pWindowHandle)
	{
		return m_base->GetWindowAssociation(pWindowHandle);
	}


	HRESULT STDMETHODCALLTYPE DXGIFactory::MakeWindowAssociation(HWND WindowHandle, UINT Flags)
	{
		return m_base->MakeWindowAssociation(WindowHandle, Flags);
	}


	BOOL STDMETHODCALLTYPE DXGIFactory::IsCurrent()
	{
		return m_base->IsCurrent();
	}
}
