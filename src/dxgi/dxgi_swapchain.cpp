#include "dxgi_swapchain.h"

#include "d3d10_1_texture.h"

namespace dxup
{
	DXGISwapChain::DXGISwapChain(IDXGISwapChain* pBase)
	{
		SetBase(pBase);
	}
	HRESULT DXGISwapChain::GetDevice(REFIID riid, void ** ppDevice)
	{
		return m_base->GetDevice(riid, ppDevice);
	}
	HRESULT DXGISwapChain::Present(UINT SyncInterval, UINT Flags)
	{
		return m_base->Present(SyncInterval, Flags);
	}
	HRESULT DXGISwapChain::GetBuffer(UINT Buffer, REFIID riid, void ** ppSurface)
	{
		if (riid == __uuidof(ID3D10Texture2D))
		{
			ID3D11Texture2D* pBackBuffer;
			HRESULT result = m_base->GetBuffer(Buffer, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);

			*ppSurface = (void*) LookupOrCreateFromD3D11<ID3D10Texture2D, ID3D11Texture2D, D3D10Texture2D>(pBackBuffer);

			return result;
		}

		return m_base->GetBuffer(Buffer, riid, ppSurface);
	}
	HRESULT DXGISwapChain::SetFullscreenState(BOOL Fullscreen, IDXGIOutput * pTarget)
	{
		return m_base->SetFullscreenState(Fullscreen, pTarget);
	}
	HRESULT DXGISwapChain::GetFullscreenState(BOOL * pFullscreen, IDXGIOutput ** ppTarget)
	{
		return m_base->GetFullscreenState(pFullscreen, ppTarget);
	}
	HRESULT DXGISwapChain::GetDesc(DXGI_SWAP_CHAIN_DESC * pDesc)
	{
		return m_base->GetDesc(pDesc);
	}
	HRESULT DXGISwapChain::ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
	{
		return m_base->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
	}
	HRESULT DXGISwapChain::ResizeTarget(const DXGI_MODE_DESC * pNewTargetParameters)
	{
		return m_base->ResizeTarget(pNewTargetParameters);
	}
	HRESULT DXGISwapChain::GetContainingOutput(IDXGIOutput ** ppOutput)
	{
		return m_base->GetContainingOutput(ppOutput);
	}
	HRESULT DXGISwapChain::GetFrameStatistics(DXGI_FRAME_STATISTICS * pStats)
	{
		return m_base->GetFrameStatistics(pStats);
	}
	HRESULT DXGISwapChain::GetLastPresentCount(UINT * pLastPresentCount)
	{
		return m_base->GetLastPresentCount(pLastPresentCount);
	}
	HRESULT DXGISwapChain::SetPrivateData(REFGUID Name, UINT DataSize, const void * pData)
	{
		return m_base->SetPrivateData(Name, DataSize, pData);
	}
	HRESULT DXGISwapChain::SetPrivateDataInterface(REFGUID Name, const IUnknown * pUnknown)
	{
		return m_base->SetPrivateDataInterface(Name, pUnknown);
	}
	HRESULT DXGISwapChain::GetPrivateData(REFGUID Name, UINT * pDataSize, void * pData)
	{
		return m_base->GetPrivateData(Name, pDataSize, pData);
	}
	HRESULT DXGISwapChain::GetParent(REFIID riid, void ** ppParent)
	{
		return m_base->GetParent(riid, ppParent);
	}
	HRESULT DXGISwapChain::QueryInterface(REFIID riid, void** ppvObject)
	{
		return m_base->QueryInterface(riid, ppvObject);
	}
}
