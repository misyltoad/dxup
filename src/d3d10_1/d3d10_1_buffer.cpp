#include "d3d10_1_buffer.h"
#include "d3d10_1_device.h"

namespace dxup {

	D3D10Buffer::D3D10Buffer(const D3D10_BUFFER_DESC* pDesc, D3D10Device* pDevice, ID3D11Buffer* pBuffer)
	{
		this->m_desc = *pDesc;
		this->m_device = pDevice;
		this->SetBase(pBuffer);
	}

	HRESULT STDMETHODCALLTYPE D3D10Buffer::QueryInterface(REFIID riid, void** ppvObject)
	{
		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)
			|| riid == __uuidof(ID3D10DeviceChild)
			|| riid == __uuidof(ID3D10Resource)
			|| riid == __uuidof(ID3D10Buffer)) 
		{
			this->AddRef();
			*ppvObject = this->m_base;
			return S_OK;
		}

		DXUP_Log(Warn, "Couldn't find interface!");
		return E_FAIL;
	}


	UINT STDMETHODCALLTYPE D3D10Buffer::GetEvictionPriority()
	{
		return this->m_base->GetEvictionPriority();
	}


	void STDMETHODCALLTYPE D3D10Buffer::SetEvictionPriority(UINT EvictionPriority)
	{
		return this->m_base->SetEvictionPriority(EvictionPriority);
	}


	void STDMETHODCALLTYPE D3D10Buffer::GetType(D3D10_RESOURCE_DIMENSION* pResourceDimension)
	{
		this->m_base->GetType((D3D11_RESOURCE_DIMENSION*)(pResourceDimension));
	}

	HRESULT STDMETHODCALLTYPE D3D10Buffer::Map(D3D10_MAP MapType, UINT MapFlags, void ** ppData)
	{
		D3D11_MAPPED_SUBRESOURCE subres;
		subres.pData = nullptr;
		HRESULT result = this->m_device->GetD3D11Context()->Map(this->m_base, 0, (D3D11_MAP)MapType, MapFlags, &subres);
		if (ppData)
			*ppData = subres.pData;
		return result;
	}

	void STDMETHODCALLTYPE D3D10Buffer::Unmap(void)
	{
		this->m_device->GetD3D11Context()->Unmap(this->m_base, 0);
	}
}
