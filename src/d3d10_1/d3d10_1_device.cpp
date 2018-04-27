#include <algorithm>
#include <cstring>

#include "d3d10_1_include.h"
#include "d3d10_1_buffer.h"
#include "d3d10_1_device.h"
#include "d3d10_1_input_layout.h"
#include "d3d10_1_shader.h"
#include "d3d10_1_state.h"
#include "d3d10_1_query.h"
#include "d3d10_1_shader.h"
#include "d3d10_1_view_srv.h"
#include "d3d10_1_view_rtv.h"
#include "d3d10_1_blend.h"
#include "d3d10_1_view_dsv.h"
#include "d3d10_1_texture.h"
#include "d3d10_1_state.h"

namespace dxup
{
	std::unordered_map<void*, void*> D3D11ToD3D10InterfaceMap;

	D3D10Device::D3D10Device(ID3D11Device1* pD3D11Device)
	{
		SetBase(pD3D11Device);
		m_base->GetImmediateContext1(&m_context);

		TextFilterSize[0] = 0;
		TextFilterSize[1] = 0;
	}

	HRESULT STDMETHODCALLTYPE D3D10Device::QueryInterface(REFIID riid, void** ppvObject)
	{
		if (riid == __uuidof(ID3D10Device)
			|| riid == __uuidof(ID3D10Device1)
			|| riid == __uuidof(IUnknown)
			|| riid == __uuidof(IDXGIObject))
		{
			AddRef();
			*ppvObject = this;
			return S_OK;
		}

		if (riid == __uuidof(IDXGIDevice) || riid == __uuidof(IDXGIDevice1) || riid == __uuidof(IDXGIDevice2))
			return m_base->QueryInterface(riid, ppvObject);

		if (riid == __uuidof(ID3D10InfoQueue) || riid == __uuidof(ID3D10Debug))
			return E_FAIL;

		DXUP_Warn(false, "Couldn't find interface!");
		return E_FAIL;
	}

	UINT FixMiscFlags(UINT DX10Flags)
	{
		UINT returnedFlags = 0;

		if (DX10Flags & D3D10_RESOURCE_MISC_GENERATE_MIPS)
			returnedFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

		if (DX10Flags & D3D10_RESOURCE_MISC_SHARED)
			returnedFlags |= D3D11_RESOURCE_MISC_SHARED;

		if (DX10Flags & D3D10_RESOURCE_MISC_TEXTURECUBE)
			returnedFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;

		if (DX10Flags & D3D10_RESOURCE_MISC_SHARED_KEYEDMUTEX)
			returnedFlags |= D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

		if (DX10Flags & D3D10_RESOURCE_MISC_GDI_COMPATIBLE)
			returnedFlags |= D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

		return returnedFlags;
	}

	HRESULT STDMETHODCALLTYPE D3D10Device::CreateBuffer(
		const D3D10_BUFFER_DESC*      pDesc,
		const D3D10_SUBRESOURCE_DATA* pInitialData,
		ID3D10Buffer**          ppBuffer)
	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.BindFlags = pDesc->BindFlags;
		bufferDesc.ByteWidth = pDesc->ByteWidth;
		bufferDesc.CPUAccessFlags = pDesc->CPUAccessFlags;
		bufferDesc.MiscFlags = FixMiscFlags(pDesc->MiscFlags);
		bufferDesc.StructureByteStride = 0;
		bufferDesc.Usage = D3D11_USAGE(pDesc->Usage);

		ID3D11Buffer* buffer;
		HRESULT result = m_base->CreateBuffer(&bufferDesc, (const D3D11_SUBRESOURCE_DATA*)pInitialData, &buffer);

		if (buffer)
		{
			auto* buf = new D3D10Buffer(pDesc, this, buffer);;
			if (ppBuffer)
				*ppBuffer = buf;
		}

		return result;
	}

	HRESULT STDMETHODCALLTYPE D3D10Device::CreateTexture1D(
		const D3D10_TEXTURE1D_DESC*   pDesc,
		const D3D10_SUBRESOURCE_DATA* pInitialData,
		ID3D10Texture1D**       ppTexture1D)
	{
		ID3D11Texture1D* texture;

		D3D11_TEXTURE1D_DESC desc;
		std::memcpy(&desc, pDesc, sizeof(D3D10_TEXTURE1D_DESC));
		desc.MiscFlags = FixMiscFlags(pDesc->MiscFlags);

		HRESULT result = m_base->CreateTexture1D(&desc, (const D3D11_SUBRESOURCE_DATA*)pInitialData, &texture);

		if (texture)
		{
			auto* tex = new D3D10Texture1D(pDesc, this, texture);
			if (ppTexture1D)
				*ppTexture1D = tex;
		}

		return result;
	}

	HRESULT STDMETHODCALLTYPE D3D10Device::CreateTexture2D(
		const D3D10_TEXTURE2D_DESC*   pDesc,
		const D3D10_SUBRESOURCE_DATA* pInitialData,
		ID3D10Texture2D**       ppTexture2D)
	{
		ID3D11Texture2D* texture;

		D3D11_TEXTURE2D_DESC desc;
		std::memcpy(&desc, pDesc, sizeof(D3D10_TEXTURE2D_DESC));
		desc.MiscFlags = FixMiscFlags(pDesc->MiscFlags);

		HRESULT result = m_base->CreateTexture2D(&desc, (const D3D11_SUBRESOURCE_DATA*)pInitialData, &texture);

		if (texture)
		{
			auto* tex = new D3D10Texture2D(pDesc, this, texture);

			if (ppTexture2D)
				*ppTexture2D = tex;
		}

		return result;
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CreateTexture3D(
		const D3D10_TEXTURE3D_DESC*   pDesc,
		const D3D10_SUBRESOURCE_DATA* pInitialData,
		ID3D10Texture3D**       ppTexture3D)
	{
		ID3D11Texture3D* texture;

		D3D11_TEXTURE3D_DESC desc;
		std::memcpy(&desc, pDesc, sizeof(D3D10_TEXTURE3D_DESC));
		desc.MipLevels = FixMiscFlags(pDesc->MiscFlags);

		HRESULT result = m_base->CreateTexture3D((const D3D11_TEXTURE3D_DESC*)pDesc, (const D3D11_SUBRESOURCE_DATA*)pInitialData, &texture);

		if (texture)
		{
			auto* tex = new D3D10Texture3D(pDesc, this, texture);

			if (ppTexture3D)
				*ppTexture3D = tex;
		}

		return result;
	}

	HRESULT STDMETHODCALLTYPE D3D10Device::CreateShaderResourceView1(
		ID3D10Resource*                   pResource,
		const D3D10_SHADER_RESOURCE_VIEW_DESC1*  pDesc,
		ID3D10ShaderResourceView1**        ppSRView)
	{
		ID3D11Resource* d3d11Resource = ResolveResource(pResource);

		if (!d3d11Resource)
			return E_INVALIDARG;

		ID3D11ShaderResourceView* resourceView;
		HRESULT result = m_base->CreateShaderResourceView(d3d11Resource, (const D3D11_SHADER_RESOURCE_VIEW_DESC*) pDesc, &resourceView);

		if (resourceView)
		{
			D3D10ShaderResourceView* pDXUPResourceView = new D3D10ShaderResourceView(pDesc, this, resourceView, pResource);
			if (ppSRView)
				*ppSRView = pDXUPResourceView;
		}

		return result;
	}

	HRESULT STDMETHODCALLTYPE D3D10Device::CreateShaderResourceView(
		ID3D10Resource*                   pResource,
		const D3D10_SHADER_RESOURCE_VIEW_DESC*  pDesc,
		ID3D10ShaderResourceView**        ppSRView)
	{
		return CreateShaderResourceView1(pResource, (const D3D10_SHADER_RESOURCE_VIEW_DESC1*)pDesc, (ID3D10ShaderResourceView1**)ppSRView);
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CreateRenderTargetView(
		ID3D10Resource*                   pResource,
		const D3D10_RENDER_TARGET_VIEW_DESC*    pDesc,
		ID3D10RenderTargetView**          ppRTView)
	{
		ID3D11Resource* d3d11Resource = ResolveResource(pResource);

		if (!d3d11Resource)
			return E_INVALIDARG;

		ID3D11RenderTargetView* rtView;
		HRESULT result = m_base->CreateRenderTargetView(d3d11Resource, reinterpret_cast<const D3D11_RENDER_TARGET_VIEW_DESC*>(pDesc), &rtView);

		if (rtView)
		{
			auto* rt = new D3D10RenderTargetView(pDesc, this, rtView, pResource);

			if (ppRTView)
				*ppRTView = rt;
		}
		return result;
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CreateDepthStencilView(
		ID3D10Resource*                   pResource,
		const D3D10_DEPTH_STENCIL_VIEW_DESC*    pDesc,
		ID3D10DepthStencilView**          ppDepthStencilView)
	{
		ID3D11Resource* d3d11Resource = ResolveResource(pResource);

		if (!d3d11Resource)
			return E_INVALIDARG;

		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		desc.Format = pDesc->Format;
		desc.ViewDimension = D3D11_DSV_DIMENSION(pDesc->ViewDimension);
		desc.Flags = 0;
		std::memcpy( ((char*)&desc.Flags) + sizeof(UINT), ((char*)&pDesc->ViewDimension) + sizeof(D3D10_DSV_DIMENSION), sizeof(D3D10_TEX2DMS_DSV));

		ID3D11DepthStencilView* dsView;
		HRESULT result = m_base->CreateDepthStencilView(d3d11Resource, &desc, &dsView);
		
		if (dsView)
		{
			auto* dsv = new D3D10DepthStencilView(pDesc, this, dsView, pResource);

			if (ppDepthStencilView)
				*ppDepthStencilView = dsv;
		}
		return result;
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CreateInputLayout(
		const D3D10_INPUT_ELEMENT_DESC*   pInputElementDescs,
		UINT                        NumElements,
		const void*                       pShaderBytecodeWithInputSignature,
		SIZE_T                      BytecodeLength,
		ID3D10InputLayout**         ppInputLayout)
	{
		ID3D11InputLayout* inputLayout;
		HRESULT result = m_base->CreateInputLayout((const D3D11_INPUT_ELEMENT_DESC*)pInputElementDescs, NumElements, pShaderBytecodeWithInputSignature, BytecodeLength, &inputLayout);

		if (inputLayout)
		{
			auto* il = new D3D10InputLayout(this, inputLayout);;

			if (ppInputLayout)
				*ppInputLayout = il;
		}
		return result;
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CreateVertexShader(
		const void*                       pShaderBytecode,
		SIZE_T                      BytecodeLength,
		ID3D10VertexShader**        ppVertexShader)
	{
		ID3D11VertexShader* dx11Shader;
		HRESULT result = m_base->CreateVertexShader(pShaderBytecode, BytecodeLength, nullptr, &dx11Shader);

		if (dx11Shader)
		{
			auto* vs = new D3D10VertexShader(this, dx11Shader);
			if (ppVertexShader)
				*ppVertexShader = vs;
		}
		return result;
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CreateGeometryShader(
		const void*                       pShaderBytecode,
		SIZE_T                      BytecodeLength,
		ID3D10GeometryShader**      ppGeometryShader)
	{
		ID3D11GeometryShader* dx11Shader;
		HRESULT result = m_base->CreateGeometryShader(pShaderBytecode, BytecodeLength, nullptr, &dx11Shader);

		if (dx11Shader)
		{
			auto* gs = new D3D10GeometryShader(this, dx11Shader);

			if (ppGeometryShader)
				*ppGeometryShader = gs;
		}
		return result;
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CreateGeometryShaderWithStreamOutput(
		const void*                       pShaderBytecode,
		SIZE_T                      BytecodeLength,
		const D3D10_SO_DECLARATION_ENTRY* pSODeclaration,
		UINT                        NumEntries,
		UINT						OutputStreamStride,
		ID3D10GeometryShader**      ppGeometryShader)
	{
		ID3D11GeometryShader* dx11Shader;

		std::vector<D3D11_SO_DECLARATION_ENTRY> entries;

		for (int i = 0; i < NumEntries; i++)
		{
			D3D11_SO_DECLARATION_ENTRY entry;
			entry.ComponentCount = pSODeclaration[i].ComponentCount;
			entry.OutputSlot = pSODeclaration[i].OutputSlot;
			entry.SemanticIndex = pSODeclaration[i].SemanticIndex;
			entry.SemanticName = pSODeclaration[i].SemanticName;
			entry.StartComponent = pSODeclaration[i].StartComponent;
			entry.Stream = 0;
			entries.push_back(entry);
		}

		HRESULT result = m_base->CreateGeometryShaderWithStreamOutput(pShaderBytecode, BytecodeLength, entries.data(), NumEntries, &OutputStreamStride, 1, D3D11_SO_NO_RASTERIZED_STREAM, nullptr, &dx11Shader);
		if (dx11Shader)
		{
			auto* gs = new D3D10GeometryShader(this, dx11Shader);

			if (ppGeometryShader)
				*ppGeometryShader = gs;
		}
		return result;
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CreatePixelShader(
		const void*                       pShaderBytecode,
		SIZE_T                      BytecodeLength,
		ID3D10PixelShader**         ppPixelShader) {
		ID3D11PixelShader* dx11Shader;
		HRESULT result = m_base->CreatePixelShader(pShaderBytecode, BytecodeLength, nullptr, &dx11Shader);
		if (dx11Shader)
		{
			auto* ps = new D3D10PixelShader(this, dx11Shader);

			if (ppPixelShader)
				*ppPixelShader = ps;
		}
		return result;
	}

	HRESULT STDMETHODCALLTYPE D3D10Device::CreateBlendState(
		const D3D10_BLEND_DESC*           pBlendStateDesc,
		ID3D10BlendState**          ppBlendState)
	{
		D3D10_BLEND_DESC1 desc;
		desc.AlphaToCoverageEnable = pBlendStateDesc->AlphaToCoverageEnable;
		desc.IndependentBlendEnable = false;

		for (int i = 0; i < 8; i++)
		{
			desc.RenderTarget[i].BlendEnable = pBlendStateDesc->BlendEnable[i];
			desc.RenderTarget[i].BlendOp = pBlendStateDesc->BlendOp;
			desc.RenderTarget[i].BlendOpAlpha = pBlendStateDesc->BlendOpAlpha;
			desc.RenderTarget[i].DestBlend = pBlendStateDesc->DestBlend;
			desc.RenderTarget[i].DestBlendAlpha = pBlendStateDesc->DestBlendAlpha;
			desc.RenderTarget[i].RenderTargetWriteMask = pBlendStateDesc->RenderTargetWriteMask[i];
			desc.RenderTarget[i].SrcBlend = pBlendStateDesc->SrcBlend;
			desc.RenderTarget[i].SrcBlendAlpha = pBlendStateDesc->SrcBlendAlpha;
		}
		return CreateBlendState1(&desc, (ID3D10BlendState1**)ppBlendState);
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CreateBlendState1(
		const D3D10_BLEND_DESC1*          pBlendStateDesc,
		ID3D10BlendState1**         ppBlendState)
	{
		ID3D11BlendState1* dx11BlendState;

		D3D11_BLEND_DESC1 desc;
		if (pBlendStateDesc)
		{
			desc.AlphaToCoverageEnable = pBlendStateDesc->AlphaToCoverageEnable;
			desc.IndependentBlendEnable = pBlendStateDesc->IndependentBlendEnable;

			for (int i = 0; i < 8; i++)
			{
				desc.RenderTarget[i].BlendEnable = pBlendStateDesc->RenderTarget[i].BlendEnable;
				desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP(pBlendStateDesc->RenderTarget[i].BlendOp);
				desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP(pBlendStateDesc->RenderTarget[i].BlendOpAlpha);
				desc.RenderTarget[i].DestBlend = D3D11_BLEND(pBlendStateDesc->RenderTarget[i].DestBlend);
				desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND(pBlendStateDesc->RenderTarget[i].DestBlendAlpha);
				desc.RenderTarget[i].LogicOp = D3D11_LOGIC_OP_NOOP;
				desc.RenderTarget[i].LogicOpEnable = FALSE;
				desc.RenderTarget[i].RenderTargetWriteMask = pBlendStateDesc->RenderTarget[i].RenderTargetWriteMask;
				desc.RenderTarget[i].SrcBlend = D3D11_BLEND(pBlendStateDesc->RenderTarget[i].SrcBlend);
				desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND(pBlendStateDesc->RenderTarget[i].SrcBlendAlpha);
			}
		}

		HRESULT result = m_base->CreateBlendState1(pBlendStateDesc ? &desc : nullptr, &dx11BlendState);

		if (dx11BlendState)
		{
			auto* bs = new D3D10BlendState(pBlendStateDesc, this, dx11BlendState);

			if (ppBlendState)
				*ppBlendState = bs;
		}

		return result;
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CreateDepthStencilState(
		const D3D10_DEPTH_STENCIL_DESC*   pDepthStencilDesc,
		ID3D10DepthStencilState**   ppDepthStencilState)
	{
		ID3D11DepthStencilState* dx11State;
		HRESULT result = m_base->CreateDepthStencilState((const D3D11_DEPTH_STENCIL_DESC*)pDepthStencilDesc, &dx11State);

		if (dx11State)
		{
			auto *dss = new D3D10DepthStencilState(pDepthStencilDesc, this, dx11State);

			if (ppDepthStencilState)
				*ppDepthStencilState = dss;
		}
		return result;
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CreateRasterizerState(
		const D3D10_RASTERIZER_DESC*      pRasterizerDesc,
		ID3D10RasterizerState**     ppRasterizerState)
	{
		ID3D11RasterizerState1* dx11State;

		D3D11_RASTERIZER_DESC1 desc;
		if (pRasterizerDesc)
		{
			desc.AntialiasedLineEnable = pRasterizerDesc->AntialiasedLineEnable;
			desc.CullMode = D3D11_CULL_MODE(pRasterizerDesc->CullMode);
			desc.DepthBias = pRasterizerDesc->DepthBias;
			desc.DepthBiasClamp = pRasterizerDesc->DepthBiasClamp;
			desc.DepthClipEnable = pRasterizerDesc->DepthClipEnable;
			desc.FillMode = D3D11_FILL_MODE(pRasterizerDesc->FillMode);
			desc.ForcedSampleCount = 0;
			desc.FrontCounterClockwise = pRasterizerDesc->FrontCounterClockwise;
			desc.MultisampleEnable = pRasterizerDesc->MultisampleEnable;
			desc.ScissorEnable = pRasterizerDesc->ScissorEnable;
			desc.SlopeScaledDepthBias = pRasterizerDesc->SlopeScaledDepthBias;
		}

		HRESULT result = m_base->CreateRasterizerState1(pRasterizerDesc ? &desc : nullptr, &dx11State);

		if (dx11State)
		{
			auto* rs = new D3D10RasterizerState(pRasterizerDesc, this, dx11State);;

			if (ppRasterizerState)
				*ppRasterizerState = rs;
		}
		return result;
	}

	HRESULT STDMETHODCALLTYPE D3D10Device::CreateSamplerState(
		const D3D10_SAMPLER_DESC*         pSamplerDesc,
		ID3D10SamplerState**        ppSamplerState)
	{
		ID3D11SamplerState* dx11State;
		HRESULT result = m_base->CreateSamplerState((const D3D11_SAMPLER_DESC*)pSamplerDesc, &dx11State);

		if (dx11State)
		{
			auto* ss = new D3D10SamplerState(pSamplerDesc, this, dx11State);

			if (ppSamplerState)
				*ppSamplerState = ss;
		}
		return result;
	}

	static D3D11_QUERY ConvertD3D10_QUERY(D3D10_QUERY query)
	{
		switch (query)
		{
		default:
		case D3D10_QUERY_EVENT: return D3D11_QUERY_EVENT;
		case D3D10_QUERY_OCCLUSION: return D3D11_QUERY_OCCLUSION;
		case D3D10_QUERY_TIMESTAMP: return D3D11_QUERY_TIMESTAMP;
		case D3D10_QUERY_TIMESTAMP_DISJOINT: return D3D11_QUERY_TIMESTAMP_DISJOINT;
		case D3D10_QUERY_PIPELINE_STATISTICS: return D3D11_QUERY_PIPELINE_STATISTICS;
		case D3D10_QUERY_OCCLUSION_PREDICATE: return D3D11_QUERY_OCCLUSION_PREDICATE;
		case D3D10_QUERY_SO_STATISTICS: return D3D11_QUERY_SO_STATISTICS;
		case D3D10_QUERY_SO_OVERFLOW_PREDICATE: return D3D11_QUERY_SO_OVERFLOW_PREDICATE;
		}
	}

	// Todo - Josh share code between query & predicate
	HRESULT STDMETHODCALLTYPE D3D10Device::CreateQuery(
		const D3D10_QUERY_DESC*           pQueryDesc,
		ID3D10Query**               ppQuery)
	{
		ID3D11Query* dx11Query;

		D3D11_QUERY_DESC queryDesc;

		if (pQueryDesc)
		{
			queryDesc.MiscFlags = FixMiscFlags(pQueryDesc->MiscFlags);
			queryDesc.Query = ConvertD3D10_QUERY(pQueryDesc->Query);
		}

		HRESULT result = m_base->CreateQuery(pQueryDesc ? &queryDesc : nullptr, &dx11Query);

		if (dx11Query)
		{
			auto* query = new D3D10Query(pQueryDesc, this, dx11Query);
			if (ppQuery)
				*ppQuery = query;
		}
		return result;
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CreatePredicate(
		const D3D10_QUERY_DESC*           pPredicateDesc,
		ID3D10Predicate**           ppPredicate)
	{
		ID3D11Predicate* dx11Query;

		D3D11_QUERY_DESC queryDesc;

		if (pPredicateDesc)
		{
			queryDesc.MiscFlags = FixMiscFlags(pPredicateDesc->MiscFlags);
			queryDesc.Query = ConvertD3D10_QUERY(pPredicateDesc->Query);
		}

		HRESULT result = m_base->CreatePredicate(pPredicateDesc ? &queryDesc : nullptr, &dx11Query);

		if (dx11Query)
		{
			auto* predicate = new D3D10Query(pPredicateDesc, this, dx11Query);
			if (ppPredicate)
				*ppPredicate = predicate;
		}
		return result;
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CreateCounter(
		const D3D10_COUNTER_DESC*         pCounterDesc,
		ID3D10Counter**             ppCounter)
	{
		// Todo - Josh
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE D3D10Device::OpenSharedResource(
		HANDLE      hResource,
		REFIID      ReturnedInterface,
		void**      ppResource)
	{
		// Look into me. - Josh
		return m_base->OpenSharedResource(hResource, ReturnedInterface, ppResource);
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::OpenSharedResource1(
		HANDLE      hResource,
		REFIID      ReturnedInterface,
		void**      ppResource)
	{
		// Look into me. - Josh
		return m_base->OpenSharedResource1(hResource, ReturnedInterface, ppResource);
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::OpenSharedResourceByName(
		LPCWSTR     lpName,
		DWORD       dwDesiredAccess,
		REFIID      returnedInterface,
		void**      ppResource)
	{
		// Look into me. - Josh
		return m_base->OpenSharedResourceByName(lpName, dwDesiredAccess, returnedInterface, ppResource);
	}

	HRESULT STDMETHODCALLTYPE D3D10Device::CheckFormatSupport(
		DXGI_FORMAT Format,
		UINT*       pFormatSupport)
	{
		return m_base->CheckFormatSupport(Format, pFormatSupport);
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CheckMultisampleQualityLevels(
		DXGI_FORMAT Format,
		UINT        SampleCount,
		UINT*       pNumQualityLevels)
	{
		return m_base->CheckMultisampleQualityLevels(Format, SampleCount, pNumQualityLevels);
	}


	void STDMETHODCALLTYPE D3D10Device::CheckCounterInfo(D3D10_COUNTER_INFO* pCounterInfo)
	{
		// Todo Later - Josh
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::CheckCounter(
		const D3D10_COUNTER_DESC* pDesc,
		D3D10_COUNTER_TYPE* pType,
		UINT*               pActiveCounters,
		LPSTR               szName,
		UINT*               pNameLength,
		LPSTR               szUnits,
		UINT*               pUnitsLength,
		LPSTR               szDescription,
		UINT*               pDescriptionLength)
	{
		// Todo Later - Josh
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE D3D10Device::GetPrivateData(
		REFGUID guid, UINT* pDataSize, void* pData)
	{
		return m_base->GetPrivateData(guid, pDataSize, pData);
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::SetPrivateData(
		REFGUID guid, UINT DataSize, const void* pData)
	{
		return m_base->SetPrivateData(guid, DataSize, pData);
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::SetPrivateDataInterface(
		REFGUID guid, const IUnknown* pData)
	{
		return m_base->SetPrivateDataInterface(guid, pData);
	}


	D3D10_FEATURE_LEVEL1 STDMETHODCALLTYPE D3D10Device::GetFeatureLevel()
	{
		return D3D10_FEATURE_LEVEL_10_1;
	}


	UINT STDMETHODCALLTYPE D3D10Device::GetCreationFlags()
	{
		return m_base->GetCreationFlags();
	}


	HRESULT STDMETHODCALLTYPE D3D10Device::GetDeviceRemovedReason() {
		return m_base->GetDeviceRemovedReason();
	}

	HRESULT STDMETHODCALLTYPE D3D10Device::SetExceptionMode(UINT RaiseFlags)
	{
		return m_base->SetExceptionMode(RaiseFlags);
	}


	UINT STDMETHODCALLTYPE D3D10Device::GetExceptionMode()
	{
		return m_base->GetExceptionMode();
	}

	void STDMETHODCALLTYPE D3D10Device::VSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D10Buffer *const * ppConstantBuffers)
	{
		SetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers, DXUP_VERTEX_SHADER);
	}
	void STDMETHODCALLTYPE D3D10Device::PSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D10ShaderResourceView *const * ppShaderResourceViews)
	{
		SetShaderResources(StartSlot, NumViews, ppShaderResourceViews, DXUP_PIXEL_SHADER);
	}
	void STDMETHODCALLTYPE D3D10Device::PSSetShader(ID3D10PixelShader* pShader)
	{
		m_context->PSSetShader(((D3D10PixelShader*)pShader)->GetD3D11Interface(), nullptr, 0);
	}
	void STDMETHODCALLTYPE D3D10Device::PSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D10SamplerState *const * ppSamplers)
	{
		SetSamplers(StartSlot, NumSamplers, ppSamplers, DXUP_PIXEL_SHADER);
	}
	void STDMETHODCALLTYPE D3D10Device::VSSetShader(ID3D10VertexShader* pShader)
	{
		m_context->VSSetShader(((D3D10VertexShader*)pShader)->GetD3D11Interface(), nullptr, 0);
	}
	void STDMETHODCALLTYPE D3D10Device::DrawIndexed(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
	{
		m_context->DrawIndexed(IndexCount, StartIndexLocation, BaseVertexLocation);
	}
	void STDMETHODCALLTYPE D3D10Device::Draw(UINT VertexCount, UINT StartVertexLocation)
	{
		m_context->Draw(VertexCount, StartVertexLocation);
	}
	void STDMETHODCALLTYPE D3D10Device::PSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D10Buffer *const * ppConstantBuffers)
	{
		SetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers, DXUP_PIXEL_SHADER);
	}
	void STDMETHODCALLTYPE D3D10Device::IASetInputLayout(ID3D10InputLayout * pLayout)
	{
		auto* dxupInputLayout = static_cast<D3D10InputLayout*>(pLayout);
		m_context->IASetInputLayout(dxupInputLayout->GetD3D11Interface());
	}
	void STDMETHODCALLTYPE D3D10Device::IASetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D10Buffer *const * ppVertexBuffers, const UINT * pStrides, const UINT * pOffsets)
	{
		std::vector<ID3D11Buffer*> buffers;
		buffers.reserve(NumBuffers);
		for (UINT i = 0; i < NumBuffers; i++)
		{
			auto* dxupBuffer = static_cast<D3D10Buffer*>(ppVertexBuffers[i]);
			buffers.push_back(dxupBuffer->GetD3D11Interface());
		}

		m_context->IASetVertexBuffers(StartSlot, NumBuffers, buffers.data(), pStrides, pOffsets);
	}
	void STDMETHODCALLTYPE D3D10Device::IASetIndexBuffer(ID3D10Buffer * buffer, DXGI_FORMAT format, UINT offset)
	{
		auto* dxupBuffer = static_cast<D3D10Buffer*>(buffer);
		m_context->IASetIndexBuffer(dxupBuffer->GetD3D11Interface(), format, offset);
	}
	void STDMETHODCALLTYPE D3D10Device::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
	{
		m_context->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
	}
	void STDMETHODCALLTYPE D3D10Device::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
	{
		m_context->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
	}
	void STDMETHODCALLTYPE D3D10Device::GSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D10Buffer *const * ppConstantBuffers)
	{
		SetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers, DXUP_GEOMETRY_SHADER);
	}
	void STDMETHODCALLTYPE D3D10Device::GSSetShader(ID3D10GeometryShader* pShader)
	{
		m_context->GSSetShader(((D3D10GeometryShader*)pShader)->GetD3D11Interface(), nullptr, 0);
	}
	void STDMETHODCALLTYPE D3D10Device::IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY Topology)
	{
		m_context->IASetPrimitiveTopology(Topology);
	}
	void STDMETHODCALLTYPE D3D10Device::VSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D10ShaderResourceView *const * ppShaderResourceViews)
	{
		SetShaderResources(StartSlot, NumViews, ppShaderResourceViews, DXUP_VERTEX_SHADER);
	}
	void STDMETHODCALLTYPE D3D10Device::VSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D10SamplerState *const * ppSamplers)
	{
		SetSamplers(StartSlot, NumSamplers, ppSamplers, DXUP_VERTEX_SHADER);
	}
	void STDMETHODCALLTYPE D3D10Device::SetPredication(ID3D10Predicate* pred, BOOL value)
	{
		auto* dxupQuery = static_cast<D3D10Query*>(pred);
		m_context->SetPredication(dxupQuery->GetD3D11Interface(), value);
	}
	void STDMETHODCALLTYPE D3D10Device::GSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D10ShaderResourceView *const * ppShaderResourceViews)
	{
		SetShaderResources(StartSlot, NumViews, ppShaderResourceViews, DXUP_GEOMETRY_SHADER);
	}
	void STDMETHODCALLTYPE D3D10Device::GSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D10SamplerState *const * ppSamplers)
	{
		SetSamplers(StartSlot, NumSamplers, ppSamplers, DXUP_GEOMETRY_SHADER);
	}
	void STDMETHODCALLTYPE D3D10Device::OMSetRenderTargets(UINT NumViews, ID3D10RenderTargetView *const * pRTV, ID3D10DepthStencilView * pDSV)
	{
		std::vector<ID3D11RenderTargetView*> rts;
		rts.reserve(NumViews);
		for (UINT i = 0; i < NumViews; i++)
		{
			auto* dxupRTV = static_cast<D3D10RenderTargetView*>(pRTV[i]);
			rts.push_back(dxupRTV->GetD3D11Interface());
		}

		auto* dxupDSV = static_cast<D3D10DepthStencilView*>(pDSV);

		m_context->OMSetRenderTargets(NumViews, rts.data(), dxupDSV->GetD3D11Interface());
	}
	void STDMETHODCALLTYPE D3D10Device::OMSetBlendState(ID3D10BlendState* pState, const FLOAT BlendFactor[4], UINT SampleMask)
	{
		auto* dxupBlendState = static_cast<D3D10BlendState*>(pState);
		m_context->OMSetBlendState(dxupBlendState->GetD3D11Interface(), BlendFactor, SampleMask);
	}
	void STDMETHODCALLTYPE D3D10Device::OMSetDepthStencilState(ID3D10DepthStencilState* pState, UINT StencilRef)
	{
		auto* dxupDepthStencilState = static_cast<D3D10DepthStencilState*>(pState);
		m_context->OMSetDepthStencilState(dxupDepthStencilState->GetD3D11Interface(), StencilRef);
	}
	void STDMETHODCALLTYPE D3D10Device::SOSetTargets(UINT NumBuffers, ID3D10Buffer *const * ppSoTargets, const UINT * pOffsets)
	{
		std::vector<ID3D11Buffer*> soTargets;
		soTargets.reserve(NumBuffers);
		m_soOffsets.clear();
		for (UINT i = 0; i < NumBuffers; i++)
		{
			auto* dxupBuf = static_cast<D3D10Buffer*>(ppSoTargets[i]);
			soTargets.push_back(dxupBuf->GetD3D11Interface());
			m_soOffsets.push_back(pOffsets[i]);
		}

		m_context->SOSetTargets(NumBuffers, soTargets.data(), pOffsets);
	}
	void STDMETHODCALLTYPE D3D10Device::DrawAuto(void)
	{
		m_context->DrawAuto();
	}
	void STDMETHODCALLTYPE D3D10Device::RSSetState(ID3D10RasterizerState* pState)
	{
		auto* dxupState = static_cast<D3D10RasterizerState*>(pState);
		m_context->RSSetState(dxupState->GetD3D11Interface());
	}
	void STDMETHODCALLTYPE D3D10Device::RSSetViewports(UINT NumViewports, const D3D10_VIEWPORT * pViewports)
	{
		std::vector<D3D11_VIEWPORT> viewports;
		viewports.reserve(NumViewports);

		for (UINT i = 0; i < NumViewports; i++)
		{
			D3D11_VIEWPORT viewport;
			viewport.Height = (FLOAT)pViewports->Height;
			viewport.MaxDepth = pViewports->MaxDepth;
			viewport.MinDepth = pViewports->MinDepth;
			viewport.TopLeftX = (FLOAT)pViewports->TopLeftX;
			viewport.TopLeftY = (FLOAT)pViewports->TopLeftY;
			viewport.Width = (FLOAT)pViewports->Width;

			viewports.push_back(viewport);
		}

		m_context->RSSetViewports(NumViewports, viewports.data());
	}
	void STDMETHODCALLTYPE D3D10Device::RSSetScissorRects(UINT, const D3D10_RECT *)
	{
		printf("Stub: RSSetScissorRects\n");
	}
	void STDMETHODCALLTYPE D3D10Device::CopySubresourceRegion(ID3D10Resource *, UINT, UINT, UINT, UINT, ID3D10Resource *, UINT, const D3D10_BOX *)
	{
		printf("Stub: CopySubresourceRegion\n");
	}
	void STDMETHODCALLTYPE D3D10Device::CopyResource(ID3D10Resource* a, ID3D10Resource* b)
	{
		ID3D11Resource* d3d11ResourceA = ResolveResource(a);
		ID3D11Resource* d3d11ResourceB = ResolveResource(b);

		m_context->CopyResource(d3d11ResourceA, d3d11ResourceB);
	}
	void STDMETHODCALLTYPE D3D10Device::UpdateSubresource(ID3D10Resource * pDstResource, UINT DstSubresource, const D3D10_BOX * pDstBox, const void * pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch)
	{
		ID3D11Resource* pDstResource11 = ResolveResource(pDstResource);

		m_context->UpdateSubresource(pDstResource11, DstSubresource, (const D3D11_BOX*)pDstBox, pSrcData, SrcRowPitch, SrcDepthPitch);
	}
	void STDMETHODCALLTYPE D3D10Device::ClearRenderTargetView(ID3D10RenderTargetView* view, const FLOAT rgba[4])
	{
		auto* dxupRTV = static_cast<D3D10RenderTargetView*>(view);
		m_context->ClearRenderTargetView(dxupRTV->GetD3D11Interface(), rgba);
	}
	void STDMETHODCALLTYPE D3D10Device::ClearDepthStencilView(ID3D10DepthStencilView * pDSV, UINT ClearFlags, FLOAT Depth, UINT8 Stencil)
	{
		auto* dxupDSV = static_cast<D3D10DepthStencilView*>(pDSV);
		m_context->ClearDepthStencilView(dxupDSV->GetD3D11Interface(), ClearFlags, Depth, Stencil);
	}
	void STDMETHODCALLTYPE D3D10Device::GenerateMips(ID3D10ShaderResourceView* pSRV)
	{
		auto* dxupSRV = static_cast<D3D10ShaderResourceView*>(pSRV);
		m_context->GenerateMips(dxupSRV->GetD3D11Interface());
	}
	void STDMETHODCALLTYPE D3D10Device::ResolveSubresource(ID3D10Resource * pDstResource, UINT DstSubresource, ID3D10Resource * pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format)
	{
		auto* dst = ResolveResource(pDstResource);
		auto* src = ResolveResource(pSrcResource);

		m_context->ResolveSubresource(dst, DstSubresource, src, SrcSubresource, Format);
	}
	void STDMETHODCALLTYPE D3D10Device::VSGetConstantBuffers(UINT, UINT, ID3D10Buffer **)
	{
		printf("Stub: VSGetConstantBuffers\n");
	}
	void STDMETHODCALLTYPE D3D10Device::PSGetShaderResources(UINT, UINT, ID3D10ShaderResourceView **)
	{
		printf("Stub: PSGetShaderResources\n");
	}
	void STDMETHODCALLTYPE D3D10Device::PSGetShader(ID3D10PixelShader **)
	{
		printf("Stub: PSGetShader\n");
	}
	void STDMETHODCALLTYPE D3D10Device::PSGetSamplers(UINT, UINT, ID3D10SamplerState **)
	{
		printf("Stub: PSGetSamplers\n");
	}
	void STDMETHODCALLTYPE D3D10Device::VSGetShader(ID3D10VertexShader **)
	{
		printf("Stub: VSGetShader\n");
	}
	void STDMETHODCALLTYPE D3D10Device::PSGetConstantBuffers(UINT, UINT, ID3D10Buffer **)
	{
		printf("Stub: PSGetConstantBuffers\n");
	}
	void STDMETHODCALLTYPE D3D10Device::IAGetInputLayout(ID3D10InputLayout **)
	{
		printf("Stub: IAGetInputLayout\n");
	}
	void STDMETHODCALLTYPE D3D10Device::IAGetVertexBuffers(UINT, UINT, ID3D10Buffer **, UINT *, UINT *)
	{
		printf("Stub: IAGetVertexBuffers\n");
	}
	void STDMETHODCALLTYPE D3D10Device::IAGetIndexBuffer(ID3D10Buffer **, DXGI_FORMAT *, UINT *)
	{
		printf("Stub: IAGetIndexBuffer\n");
	}
	void STDMETHODCALLTYPE D3D10Device::GSGetConstantBuffers(UINT, UINT, ID3D10Buffer **)
	{
		printf("Stub: GSGetConstantBuffers\n");
	}
	void STDMETHODCALLTYPE D3D10Device::GSGetShader(ID3D10GeometryShader **)
	{
		printf("Stub: GSGetShader\n");
	}
	void STDMETHODCALLTYPE D3D10Device::IAGetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY* pTopology)
	{
		m_context->IAGetPrimitiveTopology(pTopology);
	}
	void STDMETHODCALLTYPE D3D10Device::VSGetShaderResources(UINT, UINT, ID3D10ShaderResourceView **)
	{
		printf("Stub: VSGetShaderResources\n");
	}
	void STDMETHODCALLTYPE D3D10Device::VSGetSamplers(UINT, UINT, ID3D10SamplerState **)
	{
		printf("Stub: VSGetSamplers\n");
	}
	void STDMETHODCALLTYPE D3D10Device::GetPredication(ID3D10Predicate ** ppPredicate, BOOL * pPredicateValue)
	{
		ID3D11Predicate* pDX11Predicate = nullptr;
		m_context->GetPredication(&pDX11Predicate, pPredicateValue);

		if (pDX11Predicate)
		{
			if (ppPredicate)
				*ppPredicate = LookupFromD3D11<ID3D10Predicate, ID3D11Predicate>(pDX11Predicate);
		}
	}
	void STDMETHODCALLTYPE D3D10Device::GSGetShaderResources(UINT, UINT, ID3D10ShaderResourceView **)
	{
		printf("Stub: GSGetShaderResources\n");
	}
	void STDMETHODCALLTYPE D3D10Device::GSGetSamplers(UINT, UINT, ID3D10SamplerState **)
	{
		printf("Stub: GSGetSamplers\n");
	}
	void STDMETHODCALLTYPE D3D10Device::OMGetRenderTargets(UINT NumViews, ID3D10RenderTargetView ** ppRenderTargetViews, ID3D10DepthStencilView ** ppDepthStencilView)
	{
		std::vector<ID3D11RenderTargetView*> rtvs;
		rtvs.reserve(NumViews);
		ID3D11DepthStencilView* dsv;

		m_context->OMGetRenderTargets(NumViews, rtvs.data(), &dsv);

		if (ppRenderTargetViews)
		{
			for (UINT i = 0; i < NumViews; i++)
				ppRenderTargetViews[i] = LookupFromD3D11<ID3D10RenderTargetView, ID3D11RenderTargetView>(rtvs[i]);
		}

		if (dsv)
		{
			if (ppDepthStencilView)
				*ppDepthStencilView = LookupFromD3D11<ID3D10DepthStencilView, ID3D11DepthStencilView>(dsv);
		}
	}
	void STDMETHODCALLTYPE D3D10Device::OMGetBlendState(ID3D10BlendState ** ppBlendState , FLOAT BlendFactor[4], UINT* pSampleMask)
	{
		ID3D11BlendState* pDX11BlendState;
		m_context->OMGetBlendState(&pDX11BlendState, BlendFactor, pSampleMask);

		if (ppBlendState && pDX11BlendState)
			*ppBlendState = LookupFromD3D11<ID3D10BlendState, ID3D11BlendState>(pDX11BlendState);
	}
	void STDMETHODCALLTYPE D3D10Device::OMGetDepthStencilState(ID3D10DepthStencilState ** ppStencil, UINT * StencilRef)
	{
		if (ppStencil)
			*ppStencil = nullptr;

		ID3D11DepthStencilState* dx11DepthStencil = nullptr;
		m_context->OMGetDepthStencilState(&dx11DepthStencil, StencilRef);

		if (dx11DepthStencil && ppStencil)
			*ppStencil = LookupFromD3D11<ID3D10DepthStencilState, ID3D11DepthStencilState>(dx11DepthStencil);
	}
	void STDMETHODCALLTYPE D3D10Device::SOGetTargets(UINT NumBuffers, ID3D10Buffer ** ppSOTargets, UINT * pOffsets)
	{
		std::vector<ID3D11Buffer*> dx11targets;
		std::vector<ID3D10Buffer*> dx10targets;
		dx11targets.reserve(NumBuffers);
		dx10targets.reserve(NumBuffers);

		m_context->SOGetTargets(NumBuffers, dx11targets.data());

		for (UINT i = 0; i < NumBuffers; i++)
			dx10targets.push_back(LookupFromD3D11<ID3D10Buffer, ID3D11Buffer>(dx11targets[i]));

		if (ppSOTargets)
			std::memcpy(ppSOTargets, dx10targets.data(), NumBuffers * sizeof(ID3D10Buffer*));

		if (pOffsets)
			std::memcpy(pOffsets, m_soOffsets.data(), m_soOffsets.size() * sizeof(UINT));
	}
	void STDMETHODCALLTYPE D3D10Device::RSGetState(ID3D10RasterizerState** ppState)
	{
		if (ppState)
			*ppState = nullptr;

		ID3D11RasterizerState* dx11Raster = nullptr;
		m_context->RSGetState(&dx11Raster);

		if (dx11Raster && ppState)
			*ppState = LookupFromD3D11<ID3D10RasterizerState, ID3D11RasterizerState>(dx11Raster);
	}
	void STDMETHODCALLTYPE D3D10Device::RSGetViewports(UINT* pNumViewports, D3D10_VIEWPORT* pViewports)
	{
		if (!pNumViewports)
			return;

		if (!pViewports)
		{
			m_context->RSGetViewports(pNumViewports, nullptr);
			return;
		}

		UINT viewportCount = *pNumViewports;
		std::vector<D3D11_VIEWPORT> viewports;
		viewports.reserve(viewportCount);

		m_context->RSGetViewports(pNumViewports, viewports.data());

		for (UINT i = 0; i < viewportCount; i++)
		{
			pViewports[i].Height = (UINT)viewports[i].Height;
			pViewports[i].MaxDepth = viewports[i].MaxDepth;
			pViewports[i].MinDepth = viewports[i].MinDepth;
			pViewports[i].TopLeftX = (INT)viewports[i].TopLeftX;
			pViewports[i].TopLeftY = (INT)viewports[i].TopLeftY;
			pViewports[i].Width = (UINT)viewports[i].Width;
		}
	}
	void STDMETHODCALLTYPE D3D10Device::RSGetScissorRects(UINT* pNumRects, D3D10_RECT* rects)
	{
		m_context->RSGetScissorRects(pNumRects, rects);
	}
	void STDMETHODCALLTYPE D3D10Device::ClearState(void)
	{
		m_context->ClearState();
	}
	void STDMETHODCALLTYPE D3D10Device::Flush(void)
	{
		m_context->Flush();
	}

	// From MSDN:
	// Remarks
	// This method is not implemented, and should not be used.
	void STDMETHODCALLTYPE D3D10Device::SetTextFilterSize(UINT w, UINT h)
	{
		TextFilterSize[0] = w;
		TextFilterSize[1] = h;
	}
	void STDMETHODCALLTYPE D3D10Device::GetTextFilterSize(UINT* w, UINT * h)
	{
		if (w)
			*w = TextFilterSize[0];

		if (h)
			*h = TextFilterSize[0];
	}

	ID3D11Resource * D3D10Device::ResolveResource(ID3D10Resource* pResource)
	{
		ID3D11Resource* d3d11Resource = nullptr;

		if (pResource)
		{
			D3D10_RESOURCE_DIMENSION resourceType = D3D10_RESOURCE_DIMENSION_UNKNOWN;
			pResource->GetType(&resourceType);

			switch (resourceType)
			{
			case D3D10_RESOURCE_DIMENSION_BUFFER:
				d3d11Resource = static_cast<D3D10Buffer*>(pResource)->GetD3D11Interface(); break;
			case D3D10_RESOURCE_DIMENSION_TEXTURE1D:
				d3d11Resource = static_cast<D3D10Texture1D*>(pResource)->GetD3D11Interface(); break;
			case D3D10_RESOURCE_DIMENSION_TEXTURE2D:
				d3d11Resource = static_cast<D3D10Texture2D*>(pResource)->GetD3D11Interface(); break;
			case D3D10_RESOURCE_DIMENSION_TEXTURE3D:
				d3d11Resource = static_cast<D3D10Texture3D*>(pResource)->GetD3D11Interface(); break;
			}
		}

		return d3d11Resource;
	}

	void D3D10Device::SetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D10Buffer * const * pObj, DXUP_SHADER_TYPE type)
	{
		std::vector<ID3D11Buffer*> DX11ConstantBuffers;
		DX11ConstantBuffers.reserve(NumBuffers);
		for (UINT i = 0; i < NumBuffers; i++)
		{
			auto* dxupBuffer = static_cast<D3D10Buffer*>(pObj[i]);
			DX11ConstantBuffers.push_back(dxupBuffer->GetD3D11Interface());
		}

		switch (type)
		{
		case DXUP_VERTEX_SHADER: m_context->VSSetConstantBuffers(StartSlot, NumBuffers, DX11ConstantBuffers.data()); return;
		case DXUP_PIXEL_SHADER: m_context->PSSetConstantBuffers(StartSlot, NumBuffers, DX11ConstantBuffers.data()); return;
		case DXUP_GEOMETRY_SHADER: m_context->GSSetConstantBuffers(StartSlot, NumBuffers, DX11ConstantBuffers.data()); return;
		}
	}

	void D3D10Device::SetShaderResources(UINT StartSlot, UINT NumViews, ID3D10ShaderResourceView * const * pObj, DXUP_SHADER_TYPE type)
	{
		std::vector<ID3D11ShaderResourceView*> DX11ShaderResourceViews;
		DX11ShaderResourceViews.reserve(NumViews);
		for (UINT i = 0; i < NumViews; i++)
		{
			auto* dxupSRV = static_cast<D3D10ShaderResourceView*>(pObj[i]);
			DX11ShaderResourceViews.push_back(dxupSRV->GetD3D11Interface());
		}

		switch (type)
		{
		case DXUP_VERTEX_SHADER: m_context->VSSetShaderResources(StartSlot, NumViews, DX11ShaderResourceViews.data()); return;
		case DXUP_PIXEL_SHADER: m_context->PSSetShaderResources(StartSlot, NumViews, DX11ShaderResourceViews.data()); return;
		case DXUP_GEOMETRY_SHADER: m_context->GSSetShaderResources(StartSlot, NumViews, DX11ShaderResourceViews.data()); return;
		}
	}

	void D3D10Device::SetSamplers(UINT StartSlot, UINT NumBuffers, ID3D10SamplerState * const * ppConstantBuffers, DXUP_SHADER_TYPE type)
	{
		std::vector<ID3D11SamplerState*> DX11ConstantBuffers;
		DX11ConstantBuffers.reserve(NumBuffers);
		for (UINT i = 0; i < NumBuffers; i++)
		{
			auto* dxupCB = static_cast<D3D10SamplerState*>(ppConstantBuffers[i]);
			DX11ConstantBuffers.push_back(dxupCB->GetD3D11Interface());
		}

		switch (type)
		{
		case DXUP_VERTEX_SHADER: m_context->VSSetSamplers(StartSlot, NumBuffers, DX11ConstantBuffers.data()); return;
		case DXUP_PIXEL_SHADER: m_context->PSSetSamplers(StartSlot, NumBuffers, DX11ConstantBuffers.data()); return;
		case DXUP_GEOMETRY_SHADER: m_context->GSSetSamplers(StartSlot, NumBuffers, DX11ConstantBuffers.data()); return;
		}

	}

	ID3D11DeviceContext1* D3D10Device::GetD3D11Context()
	{
		return m_context;
	}
}
