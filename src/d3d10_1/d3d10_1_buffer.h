#pragma once

#include "d3d10_1_device_child.h"
#include "d3d10_1_device.h"

namespace dxup {

	class D3D10Device;

	class D3D10Buffer : public D3D10DeviceChildDesc<D3D10_BUFFER_DESC, ID3D10Buffer, ID3D11Buffer> 
	{
	public:
		D3D10Buffer(const D3D10_BUFFER_DESC* pDesc, D3D10Device* pDevice, ID3D11Buffer* pBuffer);

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void**ppvObject);

		void STDMETHODCALLTYPE GetType(D3D10_RESOURCE_DIMENSION *pResourceDimension);

		UINT STDMETHODCALLTYPE GetEvictionPriority();

		void STDMETHODCALLTYPE SetEvictionPriority(UINT EvictionPriority);

		HRESULT STDMETHODCALLTYPE Map(D3D10_MAP MapType, UINT MapFlags, void **ppData);

		void STDMETHODCALLTYPE Unmap(void);

	};

}
