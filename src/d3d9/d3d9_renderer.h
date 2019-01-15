#pragma once

#include "d3d9_base.h"
#include "d3d9_state.h"
#include "d3d11_dynamic_buffer.h"

namespace dxup {

  class D3D9ImmediateRenderer {

  public:

    D3D9ImmediateRenderer(ID3D11Device1* device, ID3D11DeviceContext1* context, D3D9State* state);

    HRESULT Clear(DWORD Count, const D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
    HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
    HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
    HRESULT DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, const void* pIndexData, D3DFORMAT IndexDataFormat, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
    HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);

    void undirtyContext();
    void handleDepthStencilDiscard();

  private:

    bool canDraw();

    bool preDraw(); // Returns CanDraw
    void postDraw();

    void updateVertexShaderAndInputLayout();
    void updateDepthStencilState();
    void updateRasterizer();
    void updateBlendState();
    void updateSampler(uint32_t sampler);
    void updateSamplers();
    void updateTextures();
    void updateRenderTargets();
    void updatePixelShader();
    void updateVertexBuffer();
    void updateIndexBuffer();
    void updateVertexConstants();
    void updatePixelConstants();

    ID3D11Device1* m_device;
    ID3D11DeviceContext1* m_context;
    D3D9State* m_state;

    D3D11DynamicBuffer m_upVertexBuffer;
    D3D11DynamicBuffer m_upIndexBuffer;

    D3D9StateCaches m_caches;

    D3D9ConstantBuffer<false> m_vsConstants;
    D3D9ConstantBuffer<true> m_psConstants;
  };

}