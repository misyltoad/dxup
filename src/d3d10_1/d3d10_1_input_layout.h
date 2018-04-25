#pragma once

#include "d3d10_1_device_child.h"

namespace dxup
{
	class D3D10Device;

	class D3D10InputLayout : public D3D10DeviceChild<ID3D10InputLayout, ID3D11InputLayout>
	{
	public:
		D3D10InputLayout(D3D10Device* pDevice, ID3D11InputLayout* pInputLayout);

		HRESULT STDMETHODCALLTYPE QueryInterface(
			REFIID  riid,
			void**  ppvObject);

	};

}
