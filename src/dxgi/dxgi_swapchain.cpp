#include "dxgi_swapchain.h"

#include "d3d10_1_texture.h"

namespace dxup
{
	DXGISwapChain::DXGISwapChain(IDXGISwapChain* pBase)
	{
		this->SetBase(pBase);
	}
	HRESULT DXGISwapChain::GetDevice(REFIID riid, void ** ppDevice)
	{
		return this->m_base->GetDevice(riid, ppDevice);
	}
	HRESULT DXGISwapChain::Present(UINT SyncInterval, UINT Flags)
	{
		return this->m_base->Present(SyncInterval, Flags);
	}
	HRESULT DXGISwapChain::GetBuffer(UINT Buffer, REFIID riid, void ** ppSurface)
	{
		if (riid == __uuidof(ID3D10Texture2D))
		{
			ID3D11Texture2D* pBackBuffer;
			HRESULT result = this->m_base->GetBuffer(Buffer, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);

			*ppSurface = (void*) LookupOrCreateFromD3D11<ID3D10Texture2D, ID3D11Texture2D, D3D10Texture2D>(pBackBuffer);

			return result;
		}

		return this->m_base->GetBuffer(Buffer, riid, ppSurface);
	}
	HRESULT DXGISwapChain::SetFullscreenState(BOOL Fullscreen, IDXGIOutput * pTarget)
	{
		return this->m_base->SetFullscreenState(Fullscreen, pTarget);
	}
	HRESULT DXGISwapChain::GetFullscreenState(BOOL * pFullscreen, IDXGIOutput ** ppTarget)
	{
		return this->m_base->GetFullscreenState(pFullscreen, ppTarget);
	}
	HRESULT DXGISwapChain::GetDesc(DXGI_SWAP_CHAIN_DESC * pDesc)
	{
		return this->m_base->GetDesc(pDesc);
	}
	HRESULT DXGISwapChain::ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
	{
		return this->m_base->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
	}
	HRESULT DXGISwapChain::ResizeTarget(const DXGI_MODE_DESC * pNewTargetParameters)
	{
		return this->m_base->ResizeTarget(pNewTargetParameters);
	}
	HRESULT DXGISwapChain::GetContainingOutput(IDXGIOutput ** ppOutput)
	{
		return this->m_base->GetContainingOutput(ppOutput);
	}
	HRESULT DXGISwapChain::GetFrameStatistics(DXGI_FRAME_STATISTICS * pStats)
	{
		return this->m_base->GetFrameStatistics(pStats);
	}
	HRESULT DXGISwapChain::GetLastPresentCount(UINT * pLastPresentCount)
	{
		return this->m_base->GetLastPresentCount(pLastPresentCount);
	}
	HRESULT DXGISwapChain::SetPrivateData(REFGUID Name, UINT DataSize, const void * pData)
	{
		return this->m_base->SetPrivateData(Name, DataSize, pData);
	}
	HRESULT DXGISwapChain::SetPrivateDataInterface(REFGUID Name, const IUnknown * pUnknown)
	{
		return this->m_base->SetPrivateDataInterface(Name, pUnknown);
	}
	HRESULT DXGISwapChain::GetPrivateData(REFGUID Name, UINT * pDataSize, void * pData)
	{
		return this->m_base->GetPrivateData(Name, pDataSize, pData);
	}
	HRESULT DXGISwapChain::GetParent(REFIID riid, void ** ppParent)
	{
		return this->m_base->GetParent(riid, ppParent);
	}
	HRESULT DXGISwapChain::QueryInterface(REFIID riid, void** ppvObject)
	{
		return this->m_base->QueryInterface(riid, ppvObject);
	}
}
