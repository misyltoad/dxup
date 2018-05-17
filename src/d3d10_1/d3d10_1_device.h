#pragma once

#include "d3d10_1_base.h"
#include "d3d10_1_include.h"

namespace dxup
{
	class D3D10Buffer;
	class D3D10Counter;
	class D3D10DeviceContext;
	class D3D10ImmediateContext;
	class D3D10Predicate;
	class D3D10Presenter;
	class D3D10Query;
	class D3D10ShaderModule;
	class D3D10Texture1D;
	class D3D10Texture2D;
	class D3D10Texture3D;

	enum DXUP_SHADER_TYPE
	{
		DXUP_VERTEX_SHADER,
		DXUP_GEOMETRY_SHADER,
		DXUP_PIXEL_SHADER,
	};

	class D3D10Device : public D3D10Base<ID3D10Device1, ID3D11Device1>
	{
	public:

		D3D10Device(ID3D11Device1* pD3D11Device);

		HRESULT STDMETHODCALLTYPE QueryInterface(
			REFIID                  riid,
			void**                  ppvObject);

		HRESULT STDMETHODCALLTYPE CreateBuffer(
			const D3D10_BUFFER_DESC*      pDesc,
			const D3D10_SUBRESOURCE_DATA* pInitialData,
			ID3D10Buffer**          ppBuffer);

		HRESULT STDMETHODCALLTYPE CreateTexture1D(
			const D3D10_TEXTURE1D_DESC*   pDesc,
			const D3D10_SUBRESOURCE_DATA* pInitialData,
			ID3D10Texture1D**       ppTexture1D);

		HRESULT STDMETHODCALLTYPE CreateTexture2D(
			const D3D10_TEXTURE2D_DESC*   pDesc,
			const D3D10_SUBRESOURCE_DATA* pInitialData,
			ID3D10Texture2D**       ppTexture2D);

		HRESULT STDMETHODCALLTYPE CreateTexture3D(
			const D3D10_TEXTURE3D_DESC*   pDesc,
			const D3D10_SUBRESOURCE_DATA* pInitialData,
			ID3D10Texture3D**       ppTexture3D);

		HRESULT STDMETHODCALLTYPE CreateShaderResourceView(
			ID3D10Resource*                   pResource,
			const D3D10_SHADER_RESOURCE_VIEW_DESC*  pDesc,
			ID3D10ShaderResourceView**        ppSRView);

		HRESULT STDMETHODCALLTYPE CreateShaderResourceView1(
			ID3D10Resource*                   pResource,
			const D3D10_SHADER_RESOURCE_VIEW_DESC1*  pDesc,
			ID3D10ShaderResourceView1**        ppSRView);

		HRESULT STDMETHODCALLTYPE CreateRenderTargetView(
			ID3D10Resource*                   pResource,
			const D3D10_RENDER_TARGET_VIEW_DESC*    pDesc,
			ID3D10RenderTargetView**          ppRTView);

		HRESULT STDMETHODCALLTYPE CreateDepthStencilView(
			ID3D10Resource*                   pResource,
			const D3D10_DEPTH_STENCIL_VIEW_DESC*    pDesc,
			ID3D10DepthStencilView**          ppDepthStencilView);

		HRESULT STDMETHODCALLTYPE CreateInputLayout(
			const D3D10_INPUT_ELEMENT_DESC*   pInputElementDescs,
			UINT                        NumElements,
			const void*                       pShaderBytecodeWithInputSignature,
			SIZE_T                      BytecodeLength,
			ID3D10InputLayout**         ppInputLayout);

		HRESULT STDMETHODCALLTYPE CreateVertexShader(
			const void*                       pShaderBytecode,
			SIZE_T                      BytecodeLength,
			ID3D10VertexShader**        ppVertexShader);

		HRESULT STDMETHODCALLTYPE CreateGeometryShader(
			const void*                       pShaderBytecode,
			SIZE_T                      BytecodeLength,
			ID3D10GeometryShader**      ppGeometryShader);

		HRESULT STDMETHODCALLTYPE CreateGeometryShaderWithStreamOutput(
			const void*                       pShaderBytecode,
			SIZE_T                      BytecodeLength,
			const D3D10_SO_DECLARATION_ENTRY* pSODeclaration,
			UINT                        NumEntries,
			UINT						OutputStreamStride,
			ID3D10GeometryShader**      ppGeometryShader);

		HRESULT STDMETHODCALLTYPE CreatePixelShader(
			const void*                       pShaderBytecode,
			SIZE_T                      BytecodeLength,
			ID3D10PixelShader**         ppPixelShader);

		HRESULT STDMETHODCALLTYPE CreateBlendState(
			const D3D10_BLEND_DESC*           pBlendStateDesc,
			ID3D10BlendState**          ppBlendState);

		HRESULT STDMETHODCALLTYPE CreateBlendState1(
			const D3D10_BLEND_DESC1*          pBlendStateDesc,
			ID3D10BlendState1**         ppBlendState);

		HRESULT STDMETHODCALLTYPE CreateDepthStencilState(
			const D3D10_DEPTH_STENCIL_DESC*   pDepthStencilDesc,
			ID3D10DepthStencilState**   ppDepthStencilState);

		HRESULT STDMETHODCALLTYPE CreateRasterizerState(
			const D3D10_RASTERIZER_DESC*      pRasterizerDesc,
			ID3D10RasterizerState**     ppRasterizerState);


		HRESULT STDMETHODCALLTYPE CreateSamplerState(
			const D3D10_SAMPLER_DESC*         pSamplerDesc,
			ID3D10SamplerState**        ppSamplerState);

		HRESULT STDMETHODCALLTYPE CreateQuery(
			const D3D10_QUERY_DESC*           pQueryDesc,
			ID3D10Query**               ppQuery);

		HRESULT STDMETHODCALLTYPE CreatePredicate(
			const D3D10_QUERY_DESC*           pPredicateDesc,
			ID3D10Predicate**           ppPredicate);

		HRESULT STDMETHODCALLTYPE CreateCounter(
			const D3D10_COUNTER_DESC*         pCounterDesc,
			ID3D10Counter**             ppCounter);

		HRESULT STDMETHODCALLTYPE OpenSharedResource(
			HANDLE      hResource,
			REFIID      ReturnedInterface,
			void**      ppResource);

		HRESULT STDMETHODCALLTYPE OpenSharedResource1(
			HANDLE      hResource,
			REFIID      returnedInterface,
			void**      ppResource);

		HRESULT STDMETHODCALLTYPE OpenSharedResourceByName(
			LPCWSTR     lpName,
			DWORD       dwDesiredAccess,
			REFIID      returnedInterface,
			void**      ppResource);

		HRESULT STDMETHODCALLTYPE CheckFormatSupport(
			DXGI_FORMAT Format,
			UINT*       pFormatSupport);

		HRESULT STDMETHODCALLTYPE CheckMultisampleQualityLevels(
			DXGI_FORMAT Format,
			UINT        SampleCount,
			UINT*       pNumQualityLevels);

		void STDMETHODCALLTYPE CheckCounterInfo(
			D3D10_COUNTER_INFO* pCounterInfo);

		HRESULT STDMETHODCALLTYPE CheckCounter(
			const D3D10_COUNTER_DESC* pDesc,
			D3D10_COUNTER_TYPE* pType,
			UINT*               pActiveCounters,
			LPSTR               szName,
			UINT*               pNameLength,
			LPSTR               szUnits,
			UINT*               pUnitsLength,
			LPSTR               szDescription,
			UINT*               pDescriptionLength);

		HRESULT STDMETHODCALLTYPE GetPrivateData(
			REFGUID Name,
			UINT    *pDataSize,
			void    *pData);

		HRESULT STDMETHODCALLTYPE SetPrivateData(
			REFGUID Name,
			UINT    DataSize,
			const void    *pData);

		HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(
			REFGUID  Name,
			const IUnknown *pUnknown);

		D3D10_FEATURE_LEVEL1 STDMETHODCALLTYPE GetFeatureLevel();

		UINT STDMETHODCALLTYPE GetCreationFlags();

		HRESULT STDMETHODCALLTYPE GetDeviceRemovedReason();

		HRESULT STDMETHODCALLTYPE SetExceptionMode(UINT RaiseFlags);

		UINT STDMETHODCALLTYPE GetExceptionMode();

		///////////////////////////////////////
		// Context
		///////////////////////////////////////

		void STDMETHODCALLTYPE VSSetConstantBuffers(UINT, UINT, ID3D10Buffer *const *);
		void STDMETHODCALLTYPE PSSetShaderResources(UINT, UINT, ID3D10ShaderResourceView *const *);
		void STDMETHODCALLTYPE PSSetShader(ID3D10PixelShader *);
		void STDMETHODCALLTYPE PSSetSamplers(UINT, UINT, ID3D10SamplerState *const *);
		void STDMETHODCALLTYPE VSSetShader(ID3D10VertexShader *);
		void STDMETHODCALLTYPE DrawIndexed(UINT, UINT, INT);
		void STDMETHODCALLTYPE Draw(UINT, UINT);
		void STDMETHODCALLTYPE PSSetConstantBuffers(UINT, UINT, ID3D10Buffer *const *);
		void STDMETHODCALLTYPE IASetInputLayout(ID3D10InputLayout *);
		void STDMETHODCALLTYPE IASetVertexBuffers(UINT, UINT, ID3D10Buffer *const *, const UINT *, const UINT *);
		void STDMETHODCALLTYPE IASetIndexBuffer(ID3D10Buffer *, DXGI_FORMAT, UINT);
		void STDMETHODCALLTYPE DrawIndexedInstanced(UINT, UINT, UINT, INT, UINT);
		void STDMETHODCALLTYPE DrawInstanced(UINT, UINT, UINT, UINT);
		void STDMETHODCALLTYPE GSSetConstantBuffers(UINT, UINT, ID3D10Buffer *const *);
		void STDMETHODCALLTYPE GSSetShader(ID3D10GeometryShader *);
		void STDMETHODCALLTYPE IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY);
		void STDMETHODCALLTYPE VSSetShaderResources(UINT, UINT, ID3D10ShaderResourceView *const *);
		void STDMETHODCALLTYPE VSSetSamplers(UINT, UINT, ID3D10SamplerState *const *);
		void STDMETHODCALLTYPE SetPredication(ID3D10Predicate *, BOOL);
		void STDMETHODCALLTYPE GSSetShaderResources(UINT, UINT, ID3D10ShaderResourceView *const *);
		void STDMETHODCALLTYPE GSSetSamplers(UINT, UINT, ID3D10SamplerState *const *);
		void STDMETHODCALLTYPE OMSetRenderTargets(UINT, ID3D10RenderTargetView *const *, ID3D10DepthStencilView *);
		void STDMETHODCALLTYPE OMSetBlendState(ID3D10BlendState *, const FLOAT[], UINT);
		void STDMETHODCALLTYPE OMSetDepthStencilState(ID3D10DepthStencilState *, UINT);
		void STDMETHODCALLTYPE SOSetTargets(UINT, ID3D10Buffer *const *, const UINT *);
		void STDMETHODCALLTYPE DrawAuto(void);
		void STDMETHODCALLTYPE RSSetState(ID3D10RasterizerState *);
		void STDMETHODCALLTYPE RSSetViewports(UINT, const D3D10_VIEWPORT *);
		void STDMETHODCALLTYPE RSSetScissorRects(UINT, const D3D10_RECT *);
		void STDMETHODCALLTYPE CopySubresourceRegion(ID3D10Resource *, UINT, UINT, UINT, UINT, ID3D10Resource *, UINT, const D3D10_BOX *);
		void STDMETHODCALLTYPE CopyResource(ID3D10Resource *, ID3D10Resource *);
		void STDMETHODCALLTYPE UpdateSubresource(ID3D10Resource *, UINT, const D3D10_BOX *, const void *, UINT, UINT);
		void STDMETHODCALLTYPE ClearRenderTargetView(ID3D10RenderTargetView *, const FLOAT[]);
		void STDMETHODCALLTYPE ClearDepthStencilView(ID3D10DepthStencilView *, UINT, FLOAT, UINT8);
		void STDMETHODCALLTYPE GenerateMips(ID3D10ShaderResourceView *);
		void STDMETHODCALLTYPE ResolveSubresource(ID3D10Resource *, UINT, ID3D10Resource *, UINT, DXGI_FORMAT);
		void STDMETHODCALLTYPE VSGetConstantBuffers(UINT, UINT, ID3D10Buffer **);
		void STDMETHODCALLTYPE PSGetShaderResources(UINT, UINT, ID3D10ShaderResourceView **);
		void STDMETHODCALLTYPE PSGetShader(ID3D10PixelShader **);
		void STDMETHODCALLTYPE PSGetSamplers(UINT, UINT, ID3D10SamplerState **);
		void STDMETHODCALLTYPE VSGetShader(ID3D10VertexShader **);
		void STDMETHODCALLTYPE PSGetConstantBuffers(UINT, UINT, ID3D10Buffer **);
		void STDMETHODCALLTYPE IAGetInputLayout(ID3D10InputLayout **);
		void STDMETHODCALLTYPE IAGetVertexBuffers(UINT, UINT, ID3D10Buffer **, UINT *, UINT *);
		void STDMETHODCALLTYPE IAGetIndexBuffer(ID3D10Buffer **, DXGI_FORMAT *, UINT *);
		void STDMETHODCALLTYPE GSGetConstantBuffers(UINT, UINT, ID3D10Buffer **);
		void STDMETHODCALLTYPE GSGetShader(ID3D10GeometryShader **);
		void STDMETHODCALLTYPE IAGetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY *);
		void STDMETHODCALLTYPE VSGetShaderResources(UINT, UINT, ID3D10ShaderResourceView **);
		void STDMETHODCALLTYPE VSGetSamplers(UINT, UINT, ID3D10SamplerState **);
		void STDMETHODCALLTYPE GetPredication(ID3D10Predicate **, BOOL *);
		void STDMETHODCALLTYPE GSGetShaderResources(UINT, UINT, ID3D10ShaderResourceView **);
		void STDMETHODCALLTYPE GSGetSamplers(UINT, UINT, ID3D10SamplerState **);
		void STDMETHODCALLTYPE OMGetRenderTargets(UINT, ID3D10RenderTargetView **, ID3D10DepthStencilView **);
		void STDMETHODCALLTYPE OMGetBlendState(ID3D10BlendState **, FLOAT[], UINT *);
		void STDMETHODCALLTYPE OMGetDepthStencilState(ID3D10DepthStencilState **, UINT *);
		void STDMETHODCALLTYPE SOGetTargets(UINT, ID3D10Buffer **, UINT *);
		void STDMETHODCALLTYPE RSGetState(ID3D10RasterizerState **);
		void STDMETHODCALLTYPE RSGetViewports(UINT *, D3D10_VIEWPORT *);
		void STDMETHODCALLTYPE RSGetScissorRects(UINT *, D3D10_RECT *);
		void STDMETHODCALLTYPE ClearState(void);
		void STDMETHODCALLTYPE Flush(void);
		void STDMETHODCALLTYPE SetTextFilterSize(UINT, UINT);
		void STDMETHODCALLTYPE GetTextFilterSize(UINT *, UINT *);

		ID3D11Resource* ResolveResource(ID3D10Resource* pResource);

		void SetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D10Buffer *const * pObj, DXUP_SHADER_TYPE type);
		void SetShaderResources(UINT StartSlot, UINT NumViews, ID3D10ShaderResourceView *const * pObj, DXUP_SHADER_TYPE type);
		void SetSamplers(UINT StartSlot, UINT NumBuffers, ID3D10SamplerState *const * pObj, DXUP_SHADER_TYPE type);

		ID3D11DeviceContext1* GetD3D11Context();
	private:
		ID3D11DeviceContext1* m_context;

		UINT TextFilterSize[2];
		UINT m_soOffsets[D3D11_SO_STREAM_COUNT];
	};

	template <typename IDX10, typename IDX11>
	IDX10* LookupFromD3D11(IDX11* address)
	{
		UINT DataSize = sizeof(void*);
		IDX10* returnAddress = nullptr;
		if (FAILED(address->GetPrivateData(__uuidof(D3D10Map), &DataSize, &returnAddress)))
		{
			DXUP_Log(Fail, "Failed to lookup D3D11 interface.");
			return nullptr;
		}

		return returnAddress;
	}

	template <typename IDX10, typename IDX11, typename DX10>
	IDX10* LookupOrCreateFromD3D11(IDX11* address)
	{
		UINT DataSize = sizeof(void*);
		IDX10* returnAddress;
		if (!FAILED(address->GetPrivateData(__uuidof(D3D10Map), &DataSize, &returnAddress)))
			return (IDX10*)returnAddress;

		return new DX10(address);
	}
}
