#pragma once

#include <dxgi1_3.h>

#ifndef NO_MAP
#define NO_MAP
#endif

#include "d3d10_1_base.h"

namespace dxup
{
	class DXGIFactory : public D3D10Base<IDXGIFactory2, IDXGIFactory2>
	{
	public:
		DXGIFactory(IDXGIFactory2* base);

		HRESULT STDMETHODCALLTYPE QueryInterface(
			REFIID                riid,
			void**                ppvObject) final;

		HRESULT STDMETHODCALLTYPE GetParent(
			REFIID                riid,
			void**                ppParent) final;

		HRESULT STDMETHODCALLTYPE CreateSoftwareAdapter(
			HMODULE               Module,
			IDXGIAdapter**        ppAdapter) final;

		HRESULT STDMETHODCALLTYPE CreateSwapChain(
			IUnknown*             pDevice,
			DXGI_SWAP_CHAIN_DESC* pDesc,
			IDXGISwapChain**      ppSwapChain) final;

		HRESULT STDMETHODCALLTYPE EnumAdapters(
			UINT                  Adapter,
			IDXGIAdapter**        ppAdapter) final;

		HRESULT STDMETHODCALLTYPE EnumAdapters1(
			UINT                  Adapter,
			IDXGIAdapter1**       ppAdapter) final;

		HRESULT STDMETHODCALLTYPE GetWindowAssociation(
			HWND*                 pWindowHandle) final;

		HRESULT STDMETHODCALLTYPE MakeWindowAssociation(
			HWND                  WindowHandle,
			UINT                  Flags) final;

		BOOL STDMETHODCALLTYPE IsCurrent();

		BOOL STDMETHODCALLTYPE IsWindowedStereoEnabled(void);

		HRESULT STDMETHODCALLTYPE CreateSwapChainForHwnd(
			IUnknown *pDevice,
			HWND hWnd,
			const DXGI_SWAP_CHAIN_DESC1 *pDesc,
			const DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pFullscreenDesc,
			IDXGIOutput *pRestrictToOutput,
			IDXGISwapChain1 **ppSwapChain);

		HRESULT STDMETHODCALLTYPE CreateSwapChainForCoreWindow(
			IUnknown *pDevice,
			IUnknown *pWindow,
			const DXGI_SWAP_CHAIN_DESC1 *pDesc,
			IDXGIOutput *pRestrictToOutput,
			IDXGISwapChain1 **ppSwapChain);

		HRESULT STDMETHODCALLTYPE GetSharedResourceAdapterLuid(
			HANDLE hResource,
			LUID *pLuid);

		HRESULT STDMETHODCALLTYPE RegisterStereoStatusWindow(
			HWND WindowHandle,
			UINT wMsg,
			DWORD *pdwCookie);

		HRESULT STDMETHODCALLTYPE RegisterStereoStatusEvent(
			HANDLE hEvent,
			DWORD *pdwCookie);

		void STDMETHODCALLTYPE UnregisterStereoStatus(
			DWORD dwCookie);

		HRESULT STDMETHODCALLTYPE RegisterOcclusionStatusWindow(
			HWND WindowHandle,
			UINT wMsg,
			DWORD *pdwCookie);

		HRESULT STDMETHODCALLTYPE RegisterOcclusionStatusEvent(
			HANDLE hEvent,
			DWORD *pdwCookie);

		void STDMETHODCALLTYPE UnregisterOcclusionStatus(
			DWORD dwCookie);

		HRESULT STDMETHODCALLTYPE CreateSwapChainForComposition(
			IUnknown *pDevice,
			const DXGI_SWAP_CHAIN_DESC1 *pDesc,
			IDXGIOutput *pRestrictToOutput,
			_Outptr_  IDXGISwapChain1 **ppSwapChain);
	};

}
