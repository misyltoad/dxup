#pragma once

#include "d3d10_1_device_child.h"

namespace dxup
{
	class D3D10Device;

	template <typename DX10Interface, typename DX11Interface>
	class D3D10Shader : public dxup::D3D10DeviceChild<DX10Interface, DX11Interface>
	{
	public:
		D3D10Shader(D3D10Device* device, DX11Interface* dx11)
		{
			this->m_device = device;
			this->SetBase(dx11);
		}

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
		{
			*ppvObject = nullptr;

			if (riid == __uuidof(IUnknown)
				|| riid == __uuidof(ID3D10DeviceChild)
				|| riid == __uuidof(DX10Interface))
			{
				this->AddRef();
				*ppvObject = this;
				return S_OK;
			}

			return this->m_base->QueryInterface(riid, ppvObject);
		}
	};

	using D3D10VertexShader = D3D10Shader<ID3D10VertexShader, ID3D11VertexShader>;
	using D3D10GeometryShader = D3D10Shader<ID3D10GeometryShader, ID3D11GeometryShader>;
	using D3D10PixelShader = D3D10Shader<ID3D10PixelShader, ID3D11PixelShader>;
}
