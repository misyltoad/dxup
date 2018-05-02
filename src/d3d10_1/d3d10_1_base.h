#pragma once

#include "d3d10_1_include.h"
#include "logger.h"
#include <unordered_map>
#include <map>
#include <assert.h>

namespace dxup
{
#ifdef DXUP_EXPORTS
#define DXUP_EXPORT __declspec(dllexport)
#else
#define DXUP_EXPORT __declspec(dllimport)
#endif

	MIDL_INTERFACE("907bf281-ea3c-43b4-a8e4-9f231107b4ff")
	D3D10Map
	{
	};

	template<typename DX10Interface, typename DX11Interface>
	class D3D10Unknown : public DX10Interface
	{
	public:
		void SetBase(DX11Interface* pNewBase)
		{
			m_base = pNewBase;
		}

		DX11Interface* GetBase()
		{
			return m_base;
		}

		ULONG STDMETHODCALLTYPE AddRef()
		{
			return m_base->AddRef();
		}

		ULONG STDMETHODCALLTYPE Release()
		{
			ULONG RefCount = m_base->Release();
			//if (RefCount == 0)
			//	delete this;

			return RefCount;
		}

		DX11Interface* GetD3D11Interface()
		{
			if (!this)
				return nullptr;

			return m_base;
		}

		const DX11Interface* GetD3D11Interface() const
		{
			if (!this)
				return nullptr;

			return m_base;
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
			return m_base->GetPrivateData(guid, pDataSize, pData);
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
			return m_base->SetPrivateData(guid, DataSize, pData);
		}

		HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(
			REFGUID  guid,
			const IUnknown *pUnknown)
		{
			return m_base->SetPrivateDataInterface(guid, pUnknown);
		}
	};

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
				d3d10->m_cachedResource10 = new D3D10Texture2D(pD3D11Tex);
				d3d10->m_cachedResource11 = pD3D11Tex;

				d3d10->m_cachedResource10->AddRef();
				d3d10->m_cachedResource11->AddRef();
			}

			*ppResource = d3d10->m_cachedResource10;

			DXUP_Assert(*ppResource);
		}
	}
}