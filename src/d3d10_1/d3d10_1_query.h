#pragma once

#include "d3d10_1_device_child.h"

namespace dxup
{
	class D3D10Device;

	class D3D10Query : public D3D10DeviceChildDesc<D3D10_QUERY_DESC, ID3D10Predicate, ID3D11Predicate> {

	public:
		D3D10Query(const D3D10_QUERY_DESC* pDesc, D3D10Device* pDevice, ID3D11Query* pQuery);
		D3D10Query(const D3D10_QUERY_DESC* pDesc, D3D10Device* pDevice, ID3D11Predicate* pPredicate);


		HRESULT STDMETHODCALLTYPE QueryInterface(
			REFIID  riid,
			void**  ppvObject);

		UINT STDMETHODCALLTYPE GetDataSize();

		void STDMETHODCALLTYPE Begin(void);

		void STDMETHODCALLTYPE End(void);

		HRESULT STDMETHODCALLTYPE GetData(void *pData, UINT DataSize, UINT GetDataFlags);

	};

}
