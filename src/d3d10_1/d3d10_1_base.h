#pragma once

#include "d3d10_1_include.h"
#include <unordered_map>
#include <assert.h>

namespace dxup
{
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

	extern std::unordered_map<void*, void*> D3D11ToD3D10InterfaceMap;

	template<typename DX10Interface, typename DX11Interface>
	class D3D10Unknown : public DX10Interface
	{
	public:
		~D3D10Unknown()
		{
			if (m_base)
				D3D11ToD3D10InterfaceMap.erase(m_base);
		}

		void SetBase(DX11Interface* pNewBase)
		{
			m_base = pNewBase;
			D3D11ToD3D10InterfaceMap[pNewBase] = this;
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