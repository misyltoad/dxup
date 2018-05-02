#include "d3d10_1_buffer.h"
#include "d3d10_1_device.h"

namespace dxup {

	D3D10Buffer::D3D10Buffer(const D3D10_BUFFER_DESC* pDesc, D3D10Device* pDevice, ID3D11Buffer* pBuffer)
	{
		m_desc = *pDesc;
		m_device = pDevice;
		SetBase(pBuffer);
	}

	HRESULT STDMETHODCALLTYPE D3D10Buffer::QueryInterface(REFIID riid, void** ppvObject)
	{
		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)
			|| riid == __uuidof(ID3D10DeviceChild)
			|| riid == __uuidof(ID3D10Resource)
			|| riid == __uuidof(ID3D10Buffer)) 
		{
			AddRef();
			*ppvObject = m_base;
			return S_OK;
		}

		DXUP_Log(Warn, "Couldn't find interface!");
		return E_FAIL;
	}


	UINT STDMETHODCALLTYPE D3D10Buffer::GetEvictionPriority()
	{
		return m_base->GetEvictionPriority();
	}


	void STDMETHODCALLTYPE D3D10Buffer::SetEvictionPriority(UINT EvictionPriority)
	{
		return m_base->SetEvictionPriority(EvictionPriority);
	}


	void STDMETHODCALLTYPE D3D10Buffer::GetType(D3D10_RESOURCE_DIMENSION* pResourceDimension)
	{
		m_base->GetType((D3D11_RESOURCE_DIMENSION*)(pResourceDimension));
	}

	HRESULT STDMETHODCALLTYPE D3D10Buffer::Map(D3D10_MAP MapType, UINT MapFlags, void ** ppData)
	{
		D3D11_MAPPED_SUBRESOURCE subres;
		subres.pData = nullptr;
		HRESULT result = m_device->GetD3D11Context()->Map(m_base, 0, (D3D11_MAP)MapType, MapFlags, &subres);
		if (ppData)
			*ppData = subres.pData;
		return result;
	}

	void STDMETHODCALLTYPE D3D10Buffer::Unmap(void)
	{
		m_device->GetD3D11Context()->Unmap(m_base, 0);
	}
}
