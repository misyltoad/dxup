#include "d3d10_1_device.h"
#include "d3d10_1_buffer.h"
#include "d3d10_1_texture.h"
#include "d3d10_1_view_srv.h"

#include <cstring>

namespace dxup
{
	D3D10ShaderResourceView::D3D10ShaderResourceView(const D3D10_SHADER_RESOURCE_VIEW_DESC1* pDesc, D3D10Device* pDevice, ID3D11ShaderResourceView* pSRV)
		: m_cachedResource10(nullptr)
		, m_cachedResource11(nullptr)
	{
		if (pDesc)
			this->m_desc = *pDesc;

		DXUP_Assert(pDesc); // Until I can be bothered to impl default desc.

		this->m_device = pDevice;
		this->SetBase(pSRV);
	}

	HRESULT STDMETHODCALLTYPE D3D10ShaderResourceView::QueryInterface(REFIID riid, void** ppvObject)
	{
		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)
			|| riid == __uuidof(ID3D10DeviceChild)
			|| riid == __uuidof(ID3D10View)
			|| riid == __uuidof(ID3D10ShaderResourceView)) {
			this->AddRef();
			*ppvObject = this;
			return S_OK;
		}

		DXUP_Log(Warn, "Couldn't find interface!");
		return E_FAIL;
	}

	void STDMETHODCALLTYPE D3D10ShaderResourceView::GetDesc(D3D10_SHADER_RESOURCE_VIEW_DESC* pDesc)
	{
		if (pDesc)
			std::memcpy(pDesc, &this->m_desc, sizeof(D3D10_SHADER_RESOURCE_VIEW_DESC));
	}

	void STDMETHODCALLTYPE D3D10ShaderResourceView::GetResource(ID3D10Resource** ppResource)
	{
		GetBaseResource(ppResource, this);
	}
}
