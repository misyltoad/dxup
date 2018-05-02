#include "d3d10_1_device.h"
#include "d3d10_1_buffer.h"
#include "d3d10_1_texture.h"
#include "d3d10_1_view_srv.h"

namespace dxup
{
	D3D10ShaderResourceView::D3D10ShaderResourceView(const D3D10_SHADER_RESOURCE_VIEW_DESC1* pDesc, D3D10Device* pDevice, ID3D11ShaderResourceView* pSRV, ID3D10Resource* pResource)
	{
		m_desc = *pDesc;
		m_device = pDevice;
		SetBase(pSRV);

		m_resource = pResource;
	}

	HRESULT STDMETHODCALLTYPE D3D10ShaderResourceView::QueryInterface(REFIID riid, void** ppvObject)
	{
		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)
			|| riid == __uuidof(ID3D10DeviceChild)
			|| riid == __uuidof(ID3D10View)
			|| riid == __uuidof(ID3D10ShaderResourceView)) {
			AddRef();
			*ppvObject = this;
			return S_OK;
		}

		DXUP_Log(Warn, "Couldn't find interface!");
		return E_FAIL;
	}

	void STDMETHODCALLTYPE D3D10ShaderResourceView::GetDesc(D3D10_SHADER_RESOURCE_VIEW_DESC* pDesc)
	{
		if (pDesc)
			std::memcpy(pDesc, &m_desc, sizeof(D3D10_SHADER_RESOURCE_VIEW_DESC));
	}

	void STDMETHODCALLTYPE D3D10ShaderResourceView::GetResource(ID3D10Resource** ppResource)
	{
		if (ppResource)
			*ppResource = m_resource;
	}
}
