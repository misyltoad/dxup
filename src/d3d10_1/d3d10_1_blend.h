#pragma once

#include "d3d10_1_device_child.h"

namespace dxup
{
	class D3D10Device;

	class D3D10BlendState : public D3D10DeviceChildDesc1<D3D10_BLEND_DESC1, ID3D10BlendState1, ID3D11BlendState1>
	{
	public:
		D3D10BlendState(const D3D10_BLEND_DESC1* pDesc, D3D10Device* pDevice, ID3D11BlendState1* pBlendState);

		HRESULT STDMETHODCALLTYPE QueryInterface(
			REFIID  riid,
			void**  ppvObject) final;

		void STDMETHODCALLTYPE GetDesc(
			D3D10_BLEND_DESC* pDesc) final;
	};

}
