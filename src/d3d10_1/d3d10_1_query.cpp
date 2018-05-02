#include "d3d10_1_device.h"
#include "d3d10_1_query.h"

namespace dxup
{
	D3D10Query::D3D10Query(const D3D10_QUERY_DESC* pDesc, D3D10Device* pDevice, ID3D11Query* pQuery)
	{
		SetBase((ID3D11Predicate*)pQuery);
		m_device = pDevice;
		m_desc = *pDesc;
	}

	D3D10Query::D3D10Query(const D3D10_QUERY_DESC* pDesc, D3D10Device* pDevice, ID3D11Predicate* pPredictate)
	{
		SetBase(pPredictate);
		m_device = pDevice;
		m_desc = *pDesc;
	}

	HRESULT D3D10Query::QueryInterface(REFIID riid, void** ppvObject)
	{
		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)
			|| riid == __uuidof(ID3D10DeviceChild)
			|| riid == __uuidof(ID3D10Asynchronous)
			|| riid == __uuidof(ID3D10Query))
		{
			AddRef();
			*ppvObject = this;
			return S_OK;
		}

		if (riid == __uuidof(ID3D10Predicate)
			&& m_desc.Query == D3D10_QUERY_OCCLUSION_PREDICATE) 
		{
			AddRef();
			*ppvObject = this;
			return S_OK;
		}

		DXUP_Log(Warn, "Couldn't find interface!");
		return E_FAIL;
	}

	UINT D3D10Query::GetDataSize()
	{
		return m_base->GetDataSize();
	}

	void D3D10Query::Begin(void)
	{
		m_device->GetD3D11Context()->Begin(m_base);
	}

	void D3D10Query::End(void)
	{
		m_device->GetD3D11Context()->End(m_base);
	}

	HRESULT D3D10Query::GetData(void* pData, UINT DataSize, UINT GetDataFlags)
	{
		return m_device->GetD3D11Context()->GetData(m_base, pData, DataSize, GetDataFlags);
	}
}
