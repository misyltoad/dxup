#pragma once

#include <dxgi.h>

#ifndef NO_MAP
#define NO_MAP
#endif
#include "d3d10_1_base.h"

namespace dxup
{
#ifdef DXUP_DXGI_EXPORTS
#define DXUP_DXGI_EXPORT __declspec(dllexport)
#else
#define DXUP_DXGI_EXPORT __declspec(dllimport)
#endif

	class DXUP_DXGI_EXPORT DXGISwapChain : public D3D10Unknown<IDXGISwapChain, IDXGISwapChain>
	{
	public:
		DXGISwapChain(IDXGISwapChain* pBase);

		HRESULT STDMETHODCALLTYPE GetDevice(
			REFIID riid,
			void **ppDevice);

		HRESULT STDMETHODCALLTYPE Present(
			UINT SyncInterval,
			UINT Flags);

		HRESULT STDMETHODCALLTYPE GetBuffer(
			UINT Buffer,
			REFIID riid,
			void **ppSurface);

		HRESULT STDMETHODCALLTYPE SetFullscreenState(
			BOOL Fullscreen,
			IDXGIOutput *pTarget);

		HRESULT STDMETHODCALLTYPE GetFullscreenState(
			BOOL *pFullscreen,
			IDXGIOutput **ppTarget);

		HRESULT STDMETHODCALLTYPE GetDesc(
			DXGI_SWAP_CHAIN_DESC *pDesc);

		HRESULT STDMETHODCALLTYPE ResizeBuffers(
			UINT BufferCount,
			UINT Width,
			UINT Height,
			DXGI_FORMAT NewFormat,
			UINT SwapChainFlags);

		HRESULT STDMETHODCALLTYPE ResizeTarget(
			const DXGI_MODE_DESC *pNewTargetParameters);

		HRESULT STDMETHODCALLTYPE GetContainingOutput(
			IDXGIOutput **ppOutput);

		HRESULT STDMETHODCALLTYPE GetFrameStatistics(
			DXGI_FRAME_STATISTICS *pStats);

		HRESULT STDMETHODCALLTYPE GetLastPresentCount(
			UINT *pLastPresentCount);

		HRESULT STDMETHODCALLTYPE SetPrivateData(
			REFGUID Name,
			UINT DataSize,
			const void *pData);

		HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(
			REFGUID Name,
			const IUnknown *pUnknown);

		HRESULT STDMETHODCALLTYPE GetPrivateData(
			REFGUID Name,
			UINT *pDataSize,
			void *pData);

		HRESULT STDMETHODCALLTYPE GetParent(
			REFIID riid,
			void **ppParent);

		HRESULT STDMETHODCALLTYPE QueryInterface( 
			REFIID riid,
			void **ppvObject);
	};
}