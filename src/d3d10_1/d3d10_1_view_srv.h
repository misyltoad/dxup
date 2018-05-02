#pragma once

#include "d3d10_1_device_child.h"

namespace dxup
{
	class D3D10Device;

	class D3D10ShaderResourceView : public D3D10DeviceChildDesc1<D3D10_SHADER_RESOURCE_VIEW_DESC1, ID3D10ShaderResourceView1, ID3D11ShaderResourceView>
	{
	public:

		D3D10ShaderResourceView(const D3D10_SHADER_RESOURCE_VIEW_DESC1* pDesc, D3D10Device* pDevice, ID3D11ShaderResourceView* pSRV);

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

		void STDMETHODCALLTYPE GetDesc(D3D10_SHADER_RESOURCE_VIEW_DESC* pDesc);

		void STDMETHODCALLTYPE GetResource(ID3D10Resource** ppResource);

		ID3D10Resource* m_cachedResource10;
		ID3D11Resource* m_cachedResource11;
	};

}
