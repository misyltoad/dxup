#include "dxgi_factory.h"
#include "dxgi_swapchain.h"

namespace dxup
{
	DXGIFactory::DXGIFactory(IDXGIFactory2* base)
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
		HRESULT result = m_base->CreateSwapChain(pDevice, pDesc, &originalSwapchain);

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

	BOOL DXGIFactory::IsWindowedStereoEnabled(void)
	{
		return m_base->IsWindowedStereoEnabled();
	}

	HRESULT DXGIFactory::CreateSwapChainForHwnd(IUnknown * pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1 * pDesc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC * pFullscreenDesc, IDXGIOutput * pRestrictToOutput, IDXGISwapChain1 ** ppSwapChain)
	{
		return E_NOTIMPL;
	}

	HRESULT DXGIFactory::CreateSwapChainForCoreWindow(IUnknown * pDevice, IUnknown * pWindow, const DXGI_SWAP_CHAIN_DESC1 * pDesc, IDXGIOutput * pRestrictToOutput, IDXGISwapChain1 ** ppSwapChain)
	{
		return E_NOTIMPL;
	}

	HRESULT DXGIFactory::GetSharedResourceAdapterLuid(HANDLE hResource, LUID * pLuid)
	{
		return E_NOTIMPL;
	}

	HRESULT DXGIFactory::RegisterStereoStatusWindow(HWND WindowHandle, UINT wMsg, DWORD * pdwCookie)
	{
		return m_base->RegisterStereoStatusWindow(WindowHandle, wMsg, pdwCookie);
	}

	HRESULT DXGIFactory::RegisterStereoStatusEvent(HANDLE hEvent, DWORD * pdwCookie)
	{
		return m_base->RegisterStereoStatusEvent(hEvent, pdwCookie);
	}

	void DXGIFactory::UnregisterStereoStatus(DWORD dwCookie)
	{
		m_base->UnregisterStereoStatus(dwCookie);
	}

	HRESULT DXGIFactory::RegisterOcclusionStatusWindow(HWND WindowHandle, UINT wMsg, DWORD * pdwCookie)
	{
		return E_NOTIMPL;
	}

	HRESULT DXGIFactory::RegisterOcclusionStatusEvent(HANDLE hEvent, DWORD * pdwCookie)
	{
		return E_NOTIMPL;
	}

	void DXGIFactory::UnregisterOcclusionStatus(DWORD dwCookie)
	{
	}

	HRESULT DXGIFactory::CreateSwapChainForComposition(IUnknown * pDevice, const DXGI_SWAP_CHAIN_DESC1 * pDesc, IDXGIOutput * pRestrictToOutput, IDXGISwapChain1 ** ppSwapChain)
	{
		return E_NOTIMPL;
	}

}
