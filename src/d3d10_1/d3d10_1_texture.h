#pragma once

#include "d3d10_1_device_child.h"

namespace dxup {

	class D3D10Device;

	class D3D10Texture1D : public D3D10DeviceChildDesc<D3D10_TEXTURE1D_DESC, ID3D10Texture1D, ID3D11Texture1D>
	{
	public:
		D3D10Texture1D(const D3D10_TEXTURE1D_DESC* pDesc, D3D10Device* pDevice, ID3D11Texture1D* pTexture);

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID  riid, void**  ppvObject) final;

		void STDMETHODCALLTYPE GetType(D3D10_RESOURCE_DIMENSION *pResourceDimension) final;

		UINT STDMETHODCALLTYPE GetEvictionPriority() final;

		void STDMETHODCALLTYPE SetEvictionPriority(UINT EvictionPriority) final;

		HRESULT STDMETHODCALLTYPE Map(UINT Subresource, D3D10_MAP MapType, UINT MapFlags, void **ppData) final;

		void STDMETHODCALLTYPE Unmap(UINT Subresource) final;
	};

	class DXUP_EXPORT D3D10Texture2D : public D3D10DeviceChildDesc<D3D10_TEXTURE2D_DESC, ID3D10Texture2D, ID3D11Texture2D>
	{
	public:
		D3D10Texture2D(const D3D10_TEXTURE2D_DESC* pDesc, D3D10Device* pDevice, ID3D11Texture2D* pTexture);
		D3D10Texture2D(ID3D11Texture2D* pTexture);

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID  riid, void**  ppvObject) final;

		void STDMETHODCALLTYPE GetType(D3D10_RESOURCE_DIMENSION *pResourceDimension) final;

		UINT STDMETHODCALLTYPE GetEvictionPriority() final;

		void STDMETHODCALLTYPE SetEvictionPriority(UINT EvictionPriority) final;

		HRESULT STDMETHODCALLTYPE Map(UINT Subresource, D3D10_MAP MapType, UINT MapFlags, D3D10_MAPPED_TEXTURE2D* pMappedTexture2D) final;

		void STDMETHODCALLTYPE Unmap(UINT Subresource) final;
	};

	class D3D10Texture3D : public D3D10DeviceChildDesc<D3D10_TEXTURE3D_DESC, ID3D10Texture3D, ID3D11Texture3D> 
	{
	public:
		D3D10Texture3D(const D3D10_TEXTURE3D_DESC* pDesc, D3D10Device* pDevice, ID3D11Texture3D* pTexture);

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID  riid, void**  ppvObject) final;

		void STDMETHODCALLTYPE GetType(D3D10_RESOURCE_DIMENSION *pResourceDimension) final;

		UINT STDMETHODCALLTYPE GetEvictionPriority() final;

		void STDMETHODCALLTYPE SetEvictionPriority(UINT EvictionPriority) final;

		HRESULT STDMETHODCALLTYPE Map(UINT Subresource, D3D10_MAP MapType, UINT MapFlags, D3D10_MAPPED_TEXTURE3D* pMappedTexture3D) final;

		void STDMETHODCALLTYPE Unmap(UINT Subresource) final;
	};

}
