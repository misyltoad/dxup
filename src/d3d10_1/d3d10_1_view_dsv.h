#pragma once

#include "d3d10_1_device_child.h"

namespace dxup {

	class D3D10Device;

	class D3D10DepthStencilView : public D3D10DeviceChildDesc<D3D10_DEPTH_STENCIL_VIEW_DESC, ID3D10DepthStencilView, ID3D11DepthStencilView>
	{
	public:
		D3D10DepthStencilView(const D3D10_DEPTH_STENCIL_VIEW_DESC* pDesc, D3D10Device* pDevice, ID3D11DepthStencilView* pSRV);

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

		void STDMETHODCALLTYPE GetResource(ID3D10Resource** ppResource);

		ID3D10Resource* m_cachedResource10;
		ID3D11Resource* m_cachedResource11;
	};

}
