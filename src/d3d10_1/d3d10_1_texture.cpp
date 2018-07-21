#include "d3d10_1_device.h"
#include "d3d10_1_texture.h"

namespace dxup {
	D3D10Texture1D::D3D10Texture1D(const D3D10_TEXTURE1D_DESC* pDesc, D3D10Device* pDevice, ID3D11Texture1D* pTexture)
	{
		this->m_desc = *pDesc;
		this->m_device = pDevice;
		this->SetBase(pTexture);
	}

	HRESULT STDMETHODCALLTYPE D3D10Texture1D::QueryInterface(REFIID riid, void** ppvObject)
	{
		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)
			|| riid == __uuidof(ID3D10DeviceChild)
			|| riid == __uuidof(ID3D10Resource)
			|| riid == __uuidof(ID3D10Texture1D)) {
			this->AddRef();
			*ppvObject = this;
			return S_OK;
		}

		DXUP_Log(Warn, "Couldn't find interface!");
		return E_INVALIDARG;
	}

	void STDMETHODCALLTYPE D3D10Texture1D::GetType(D3D10_RESOURCE_DIMENSION *pResourceDimension)
	{
		return this->m_base->GetType((D3D11_RESOURCE_DIMENSION*)pResourceDimension);
	}


	UINT STDMETHODCALLTYPE D3D10Texture1D::GetEvictionPriority()
	{
		return this->m_base->GetEvictionPriority();
	}


	void STDMETHODCALLTYPE D3D10Texture1D::SetEvictionPriority(UINT EvictionPriority)
	{
		this->m_base->SetEvictionPriority(EvictionPriority);
	}

	HRESULT STDMETHODCALLTYPE D3D10Texture1D::Map(UINT Subresource, D3D10_MAP MapType, UINT MapFlags, void **ppData)
	{
		D3D11_MAPPED_SUBRESOURCE subres;
		subres.pData = nullptr;
		HRESULT result = this->m_device->GetD3D11Context()->Map(this->m_base, Subresource, D3D11_MAP(MapType), MapFlags, &subres);

		DXUP_Assert(ppData);
		if (ppData)
			*ppData = subres.pData;

		DXUP_AssertSuccess(result);
		return result;
	}

	void STDMETHODCALLTYPE D3D10Texture1D::Unmap(UINT Subresource)
	{
		this->m_device->GetD3D11Context()->Unmap(this->m_base, Subresource);
	}

	D3D10Texture2D::D3D10Texture2D(const D3D10_TEXTURE2D_DESC* pDesc, D3D10Device* pDevice, ID3D11Texture2D* pTexture)
	{
		this->m_desc = *pDesc;
		this->m_device = pDevice;
		this->SetBase(pTexture);
	}

	extern UINT UnFixMiscFlags(UINT DX11Flags);

	D3D10Texture2D::D3D10Texture2D(ID3D11Texture2D* pTexture)
	{
		this->SetBase(pTexture);

		if (pTexture)
		{
			ID3D11Device* pD3D11Device = nullptr;
			pTexture->GetDevice(&pD3D11Device);
			ID3D10Device1* pD3D10Device = LookupFromD3D11<ID3D10Device1, ID3D11Device1>((ID3D11Device1*)pD3D11Device);
			this->m_device = static_cast<D3D10Device*>(pD3D10Device);
		}

		D3D11_TEXTURE2D_DESC d3d11Desc;
		pTexture->GetDesc(&d3d11Desc);

		this->m_desc.ArraySize = d3d11Desc.ArraySize;
		this->m_desc.BindFlags = d3d11Desc.BindFlags;
		this->m_desc.CPUAccessFlags = d3d11Desc.CPUAccessFlags;
		this->m_desc.Format = d3d11Desc.Format;
		this->m_desc.Height = d3d11Desc.Height;
		this->m_desc.MipLevels = d3d11Desc.MipLevels;
		this->m_desc.MiscFlags = UnFixMiscFlags(d3d11Desc.MiscFlags);
		this->m_desc.SampleDesc = d3d11Desc.SampleDesc;
		this->m_desc.Usage = D3D10_USAGE(d3d11Desc.Usage);
		this->m_desc.Width = d3d11Desc.Width;
	}

	HRESULT STDMETHODCALLTYPE D3D10Texture2D::QueryInterface(REFIID riid, void** ppvObject)
	{
		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)
			|| riid == __uuidof(ID3D10DeviceChild)
			|| riid == __uuidof(ID3D10Resource)
			|| riid == __uuidof(ID3D10Texture2D))
		{
			this->AddRef();
			*ppvObject = this;
			return S_OK;
		}

		DXUP_Log(Warn, "Couldn't find interface!");
		return E_FAIL;
	}

	void STDMETHODCALLTYPE D3D10Texture2D::GetType(D3D10_RESOURCE_DIMENSION *pResourceDimension)
	{
		this->m_base->GetType((D3D11_RESOURCE_DIMENSION*)pResourceDimension);
	}


	UINT STDMETHODCALLTYPE D3D10Texture2D::GetEvictionPriority()
	{
		return this->m_base->GetEvictionPriority();
	}


	void STDMETHODCALLTYPE D3D10Texture2D::SetEvictionPriority(UINT EvictionPriority)
	{
		this->m_base->SetEvictionPriority(EvictionPriority);
	}

	HRESULT STDMETHODCALLTYPE D3D10Texture2D::Map(UINT Subresource, D3D10_MAP MapType, UINT MapFlags, D3D10_MAPPED_TEXTURE2D* pMappedTexture2D)
	{
		D3D11_MAPPED_SUBRESOURCE subres = { 0 };
		HRESULT result = this->m_device->GetD3D11Context()->Map(this->m_base, Subresource, D3D11_MAP(MapType), MapFlags, &subres);

		DXUP_Assert(pMappedTexture2D);
		if (pMappedTexture2D)
		{
			pMappedTexture2D->pData = subres.pData;
			pMappedTexture2D->RowPitch = subres.RowPitch;
		}

		DXUP_AssertSuccess(result);
		return result;
	}

	void STDMETHODCALLTYPE D3D10Texture2D::Unmap(UINT Subresource)
	{
		this->m_device->GetD3D11Context()->Unmap(this->m_base, Subresource);
	}

	D3D10Texture3D::D3D10Texture3D(const D3D10_TEXTURE3D_DESC* pDesc, D3D10Device* pDevice, ID3D11Texture3D* pTexture)
	{
		this->m_desc = *pDesc;
		this->m_device = pDevice;
		this->SetBase(pTexture);
	}

	HRESULT STDMETHODCALLTYPE D3D10Texture3D::QueryInterface(REFIID riid, void** ppvObject) {
		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)
			|| riid == __uuidof(ID3D10DeviceChild)
			|| riid == __uuidof(ID3D10Resource)
			|| riid == __uuidof(ID3D10Texture3D))
		{
			this->AddRef();
			*ppvObject = this;
			return S_OK;
		}

		DXUP_Log(Warn, "Couldn't find interface!");
		return E_INVALIDARG;
	}


	void STDMETHODCALLTYPE D3D10Texture3D::GetType(D3D10_RESOURCE_DIMENSION *pResourceDimension)
	{
		this->m_base->GetType((D3D11_RESOURCE_DIMENSION*)pResourceDimension);
	}


	UINT STDMETHODCALLTYPE D3D10Texture3D::GetEvictionPriority()
	{
		return this->m_base->GetEvictionPriority();
	}


	void STDMETHODCALLTYPE D3D10Texture3D::SetEvictionPriority(UINT EvictionPriority)
	{
		return this->m_base->SetEvictionPriority(EvictionPriority);
	}

	HRESULT STDMETHODCALLTYPE D3D10Texture3D::Map(UINT Subresource, D3D10_MAP MapType, UINT MapFlags, D3D10_MAPPED_TEXTURE3D* pMappedTexture3D)
	{
		D3D11_MAPPED_SUBRESOURCE subres = { 0 };
		HRESULT result = this->m_device->GetD3D11Context()->Map(this->m_base, Subresource, D3D11_MAP(MapType), MapFlags, &subres);

		if (pMappedTexture3D)
		{
			pMappedTexture3D->pData = subres.pData;
			pMappedTexture3D->RowPitch = subres.RowPitch;
			pMappedTexture3D->DepthPitch = subres.DepthPitch;
		}

		DXUP_AssertSuccess(result);

		return result;
	}

	void STDMETHODCALLTYPE D3D10Texture3D::Unmap(UINT Subresource)
	{
		this->m_device->GetD3D11Context()->Unmap(this->m_base, Subresource);
	}
}
