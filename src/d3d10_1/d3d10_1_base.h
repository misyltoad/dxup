#pragma once

#include "d3d10_1_include.h"
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

#ifdef _DEBUG
	#define DXUP_Assert(condition, text, ...) \
	if (! (condition) ) \
	{ \
		printf(text "\n" , __VA_ARGS__); \
		int* foo = 0; \
		*foo = 0xDEAD; \
	}
	#define DXUP_Warn(condition, text, ...) \
	if (! (condition) ) \
	{ \
		printf(text "\n" , __VA_ARGS__); \
	}
#else
	#define DXUP_Assert(condition, text, ...)
	#define DXUP_Warn(condition, text, ...)
#endif

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
			if (RefCount == 0)
				delete this;

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

			pNewBase->SetPrivateData(__uuidof(DX10Interface), sizeof(void*), this);
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
}