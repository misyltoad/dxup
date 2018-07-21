#pragma once

#include "d3d10_1_device_child.h"

namespace dxup
{
	class D3D10Device;

	template <typename DX10DescType, typename DX10InterfaceType, typename DX11InterfaceType>
	class D3D10State : public D3D10DeviceChildDesc<DX10DescType, DX10InterfaceType, DX11InterfaceType>
	{
	public:
		D3D10State(const DX10DescType* pDesc, D3D10Device* pDevice, DX11InterfaceType* pState)
		{
			this->m_device = pDevice;
			this->SetBase(pState);
			this->m_desc = *pDesc;
		}

		HRESULT STDMETHODCALLTYPE QueryInterface(
			REFIID  riid,
			void**  ppvObject)
		{
			*ppvObject = nullptr;

			if (riid == __uuidof(IUnknown)
				|| riid == __uuidof(ID3D10DeviceChild)
				|| riid == __uuidof(DX10InterfaceType))
			{
				this->AddRef();
				*ppvObject = this;
				return S_OK;
			}

			return this->m_base->QueryInterface(riid, ppvObject);
		}
	};

	using D3D10DepthStencilState = D3D10State<D3D10_DEPTH_STENCIL_DESC, ID3D10DepthStencilState, ID3D11DepthStencilState>;
	using D3D10SamplerState = D3D10State<D3D10_SAMPLER_DESC, ID3D10SamplerState, ID3D11SamplerState>;
	using D3D10RasterizerState = D3D10State<D3D10_RASTERIZER_DESC, ID3D10RasterizerState, ID3D11RasterizerState1>;
}
