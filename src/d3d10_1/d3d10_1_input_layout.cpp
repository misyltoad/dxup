#include "d3d10_1_device.h"
#include "d3d10_1_input_layout.h"

namespace dxup
{
	D3D10InputLayout::D3D10InputLayout(D3D10Device* pDevice, ID3D11InputLayout* pInputLayout)
	{
		m_device = pDevice;
		SetBase(pInputLayout);
	}

	HRESULT STDMETHODCALLTYPE D3D10InputLayout::QueryInterface(REFIID riid, void** ppvObject)
	{
		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)
			|| riid == __uuidof(ID3D10DeviceChild)
			|| riid == __uuidof(ID3D10InputLayout))
		{
			AddRef();
			*ppvObject = this;
			return S_OK;
		}

		DXUP_Warn(false, "Couldn't find interface!");
		return E_FAIL;
	}
}