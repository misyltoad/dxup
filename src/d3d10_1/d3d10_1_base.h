#pragma once

#include "d3d10_1_include.h"
#include "logger.h"
#include <unordered_map>
#include <map>
#include <assert.h>
#include "d3d10_1_wrappers.h"

namespace dxup
{
#ifdef DXUP_EXPORTS
#define DXUP_EXPORT __declspec(dllexport)
#else
#define DXUP_EXPORT __declspec(dllimport)
#endif

#ifndef _MSC_VER
# ifdef __WINE__
#   define DXUP_DEFINE_GUID(iface) \
      template<> inline GUID const& __wine_uuidof<iface> () { return iface::guid; }
# else
#   define DXUP_DEFINE_GUID(iface) \
      template<> inline GUID const& __mingw_uuidof<iface> () { return iface::guid; }
# endif
#endif

	MIDL_INTERFACE("2ec54fe1-3185-40aa-9a32-7e634a8823f2")
	D3D10Map
	{
		static const GUID guid;
	};

	template<typename DX10Interface, typename DX11Interface>
	class D3D10Unknown : public DX10Interface
	{
	public:
		void SetBase(DX11Interface* pNewBase)
		{
			this->m_base = pNewBase;
		}

		DX11Interface* GetBase()
		{
			return this->m_base;
		}

		ULONG STDMETHODCALLTYPE AddRef()
		{
			return this->m_base->AddRef();
		}

		ULONG STDMETHODCALLTYPE Release()
		{
			ULONG RefCount = this->m_base->Release();
			if (RefCount == 0)
				delete this;

			return RefCount;
		}

		DX11Interface* GetD3D11Interface()
		{
			if (!this)
				return nullptr;

			return this->m_base;
		}

		const DX11Interface* GetD3D11Interface() const
		{
			if (!this)
				return nullptr;

			return this->m_base;
		}

	protected:
		DX11Interface* m_base;
	};

	template<typename DX10Interface, typename DX11Interface>
	class D3D10Base : public D3D10Unknown<DX10Interface, DX11Interface>
	{
	public:
		HRESULT STDMETHODCALLTYPE GetPrivateData(
			REFGUID guid,
			UINT    *pDataSize,
			void    *pData)
		{
			return this->m_base->GetPrivateData(guid, pDataSize, pData);
		}

		void SetBase(DX11Interface* pNewBase)
		{
			D3D10Unknown<DX10Interface, DX11Interface>::SetBase(pNewBase);

			pNewBase->SetPrivateData(__uuidof(D3D10Map), sizeof(void*), this);
		}

		HRESULT STDMETHODCALLTYPE SetPrivateData(
			REFGUID guid,
			UINT    DataSize,
			const void    *pData)
		{
			return this->m_base->SetPrivateData(guid, DataSize, pData);
		}

		HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(
			REFGUID  guid,
			const IUnknown *pUnknown)
		{
			return this->m_base->SetPrivateDataInterface(guid, pUnknown);
		}
	};

	template <typename D3D10, typename DXUP, typename D3D11>
	D3D11* ToD3D11Internal(D3D10* d3d10)
	{
		if (!d3d10)
			return nullptr;

		DXUP* dxup = static_cast<DXUP*>(d3d10);
		return dxup->GetD3D11Interface();
	}

#define ToD3D11(x, y) ToD3D11Internal< ID3D10##x , D3D10##x , ID3D11##x >(y)

	template <typename T>
	void GetBaseResource(ID3D10Resource** ppResource, T* d3d10)
	{
		DXUP_Assert(ppResource);
		if (ppResource)
		{
			ID3D11Resource* pD3D11Resource = nullptr;
			d3d10->GetD3D11Interface()->GetResource(&pD3D11Resource);

			ID3D11Texture2D* pD3D11Tex = reinterpret_cast<ID3D11Texture2D*>(pD3D11Resource);

			// I'm so confused. Why does this work?
			// Wish I could see Crytek's code...
			d3d10->m_cachedResource11 = pD3D11Tex;

			if (!d3d10->m_cachedResource10 || !d3d10->m_cachedResource11 || d3d10->m_cachedResource11 != pD3D11Tex)
			{
				d3d10->m_cachedResource10 = Wrap::Texture2D(pD3D11Tex);
				d3d10->m_cachedResource11 = pD3D11Tex;
			}

			*ppResource = d3d10->m_cachedResource10;

			DXUP_Assert(*ppResource);
		}
	}
}

#ifdef _MSC_VER
	namespace dxup
	{
		struct __declspec(uuid("2ec54fe1-3185-40aa-9a32-7e634a8823f2")) D3D10Map;
	}
#else
	DXUP_DEFINE_GUID(dxup::D3D10Map);
#endif