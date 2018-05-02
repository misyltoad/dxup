#include "d3d10_1_device.h"
#include "d3d10_1_buffer.h"
#include "d3d10_1_texture.h"
#include "d3d10_1_view_rtv.h"

namespace dxup
{
	D3D10RenderTargetView::D3D10RenderTargetView(const D3D10_RENDER_TARGET_VIEW_DESC* pDesc, D3D10Device* pDevice, ID3D11RenderTargetView* pRTV, ID3D10Resource* pResource)
	{
		if (pDesc)
			m_desc = *pDesc;

		m_device = pDevice;
		SetBase(pRTV);
		
		m_resource = pResource;
	}

	HRESULT STDMETHODCALLTYPE D3D10RenderTargetView::QueryInterface(REFIID riid, void** ppvObject)
	{
		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)
			|| riid == __uuidof(ID3D10DeviceChild)
			|| riid == __uuidof(ID3D10View)
			|| riid == __uuidof(ID3D10RenderTargetView)) {
			AddRef();
			*ppvObject = this;
			return S_OK;
		}

		DXUP_Log(Warn, "Couldn't find interface!");
		return E_FAIL;
	}

	void STDMETHODCALLTYPE D3D10RenderTargetView::GetResource(ID3D10Resource** ppResource)
	{
		if (ppResource)
			*ppResource = m_resource;
	}
}
