#include "d3d10_1_device.h"
#include "d3d10_1_texture.h"

namespace dxup {
	D3D10Texture1D::D3D10Texture1D(const D3D10_TEXTURE1D_DESC* pDesc, D3D10Device* pDevice, ID3D11Texture1D* pTexture)
	{
		m_desc = *pDesc;
		m_device = pDevice;
		SetBase(pTexture);
	}

	HRESULT STDMETHODCALLTYPE D3D10Texture1D::QueryInterface(REFIID riid, void** ppvObject)
	{
		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)
			|| riid == __uuidof(ID3D10DeviceChild)
			|| riid == __uuidof(ID3D10Resource)
			|| riid == __uuidof(ID3D10Texture1D)) {
			AddRef();
			*ppvObject = this;
			return S_OK;
		}

		DXUP_Log(Warn, "Couldn't find interface!");
		return E_INVALIDARG;
	}

	void STDMETHODCALLTYPE D3D10Texture1D::GetType(D3D10_RESOURCE_DIMENSION *pResourceDimension)
	{
		return m_base->GetType((D3D11_RESOURCE_DIMENSION*)pResourceDimension);
	}


	UINT STDMETHODCALLTYPE D3D10Texture1D::GetEvictionPriority()
	{
		return m_base->GetEvictionPriority();
	}


	void STDMETHODCALLTYPE D3D10Texture1D::SetEvictionPriority(UINT EvictionPriority)
	{
		m_base->SetEvictionPriority(EvictionPriority);
	}

	HRESULT STDMETHODCALLTYPE D3D10Texture1D::Map(UINT Subresource, D3D10_MAP MapType, UINT MapFlags, void **ppData)
	{
		D3D11_MAPPED_SUBRESOURCE subres;
		subres.pData = nullptr;
		HRESULT result = m_device->GetD3D11Context()->Map(m_base, Subresource, D3D11_MAP(MapType), MapFlags, &subres);
		*ppData = subres.pData;
		return result;
	}

	void STDMETHODCALLTYPE D3D10Texture1D::Unmap(UINT Subresource)
	{
		m_device->GetD3D11Context()->Unmap(m_base, Subresource);
	}

	D3D10Texture2D::D3D10Texture2D(const D3D10_TEXTURE2D_DESC* pDesc, D3D10Device* pDevice, ID3D11Texture2D* pTexture)
	{
		m_desc = *pDesc;
		m_device = pDevice;
		SetBase(pTexture);
	}

	extern UINT UnFixMiscFlags(UINT DX11Flags);

	D3D10Texture2D::D3D10Texture2D(ID3D11Texture2D* pTexture)
	{
		SetBase(pTexture);

		if (pTexture)
		{
			ID3D11Device* pD3D11Device = nullptr;
			pTexture->GetDevice(&pD3D11Device);
			ID3D10Device1* pD3D10Device = LookupFromD3D11<ID3D10Device1, ID3D11Device1>((ID3D11Device1*)pD3D11Device);
			m_device = static_cast<D3D10Device*>(pD3D10Device);
		}

		D3D11_TEXTURE2D_DESC d3d11Desc;
		pTexture->GetDesc(&d3d11Desc);

		m_desc.ArraySize = d3d11Desc.ArraySize;
		m_desc.BindFlags = d3d11Desc.BindFlags;
		m_desc.CPUAccessFlags = d3d11Desc.CPUAccessFlags;
		m_desc.Format = d3d11Desc.Format;
		m_desc.Height = d3d11Desc.Height;
		m_desc.MipLevels = d3d11Desc.MipLevels;
		m_desc.MiscFlags = UnFixMiscFlags(d3d11Desc.MiscFlags);
		m_desc.SampleDesc = d3d11Desc.SampleDesc;
		m_desc.Usage = D3D10_USAGE(d3d11Desc.Usage);
		m_desc.Width = d3d11Desc.Width;
	}

	HRESULT STDMETHODCALLTYPE D3D10Texture2D::QueryInterface(REFIID riid, void** ppvObject)
	{
		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)
			|| riid == __uuidof(ID3D10DeviceChild)
			|| riid == __uuidof(ID3D10Resource)
			|| riid == __uuidof(ID3D10Texture2D))
		{
			AddRef();
			*ppvObject = this;
			return S_OK;
		}

		DXUP_Log(Warn, "Couldn't find interface!");
		return E_FAIL;
	}

	void STDMETHODCALLTYPE D3D10Texture2D::GetType(D3D10_RESOURCE_DIMENSION *pResourceDimension)
	{
		m_base->GetType((D3D11_RESOURCE_DIMENSION*)pResourceDimension);
	}


	UINT STDMETHODCALLTYPE D3D10Texture2D::GetEvictionPriority()
	{
		return m_base->GetEvictionPriority();
	}


	void STDMETHODCALLTYPE D3D10Texture2D::SetEvictionPriority(UINT EvictionPriority)
	{
		m_base->SetEvictionPriority(EvictionPriority);
	}

	HRESULT STDMETHODCALLTYPE D3D10Texture2D::Map(UINT Subresource, D3D10_MAP MapType, UINT MapFlags, D3D10_MAPPED_TEXTURE2D* pMappedTexture2D)
	{
		D3D11_MAPPED_SUBRESOURCE subres = { 0 };
		HRESULT result = m_device->GetD3D11Context()->Map(m_base, Subresource, D3D11_MAP(MapType), MapFlags, &subres);

		pMappedTexture2D->pData = subres.pData;
		pMappedTexture2D->RowPitch = subres.RowPitch;

		return result;
	}

	void STDMETHODCALLTYPE D3D10Texture2D::Unmap(UINT Subresource)
	{
		m_device->GetD3D11Context()->Unmap(m_base, Subresource);
	}

	D3D10Texture3D::D3D10Texture3D(const D3D10_TEXTURE3D_DESC* pDesc, D3D10Device* pDevice, ID3D11Texture3D* pTexture)
	{
		m_desc = *pDesc;
		m_device = pDevice;
		SetBase(pTexture);
	}

	HRESULT STDMETHODCALLTYPE D3D10Texture3D::QueryInterface(REFIID riid, void** ppvObject) {
		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)
			|| riid == __uuidof(ID3D10DeviceChild)
			|| riid == __uuidof(ID3D10Resource)
			|| riid == __uuidof(ID3D10Texture3D))
		{
			AddRef();
			*ppvObject = this;
			return S_OK;
		}

		DXUP_Log(Warn, "Couldn't find interface!");
		return E_INVALIDARG;
	}


	void STDMETHODCALLTYPE D3D10Texture3D::GetType(D3D10_RESOURCE_DIMENSION *pResourceDimension)
	{
		m_base->GetType((D3D11_RESOURCE_DIMENSION*)pResourceDimension);
	}


	UINT STDMETHODCALLTYPE D3D10Texture3D::GetEvictionPriority()
	{
		return m_base->GetEvictionPriority();
	}


	void STDMETHODCALLTYPE D3D10Texture3D::SetEvictionPriority(UINT EvictionPriority)
	{
		return m_base->SetEvictionPriority(EvictionPriority);
	}

	HRESULT STDMETHODCALLTYPE D3D10Texture3D::Map(UINT Subresource, D3D10_MAP MapType, UINT MapFlags, D3D10_MAPPED_TEXTURE3D* pMappedTexture3D)
	{
		D3D11_MAPPED_SUBRESOURCE subres = { 0 };
		HRESULT result = m_device->GetD3D11Context()->Map(m_base, Subresource, D3D11_MAP(MapType), MapFlags, &subres);

		pMappedTexture3D->pData = subres.pData;
		pMappedTexture3D->RowPitch = subres.RowPitch;
		pMappedTexture3D->DepthPitch = subres.DepthPitch;

		return result;
	}

	void STDMETHODCALLTYPE D3D10Texture3D::Unmap(UINT Subresource)
	{
		m_device->GetD3D11Context()->Unmap(m_base, Subresource);
	}
}
