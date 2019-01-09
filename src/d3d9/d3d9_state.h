#pragma once

#include "d3d9_base.h"
#include "d3d9_device.h"
#include "d3d9_shaders.h"
#include "d3d9_vertexdeclaration.h"
#include "d3d9_buffer.h"
#include "../util/vectypes.h"

namespace dxup {

  namespace dirtyFlags {
    const uint32_t vertexShader = 1 << 0;
    const uint32_t vertexDecl = 1 << 1;
    const uint32_t pixelShader = 1 << 2;
    const uint32_t renderTargets = 1 << 3;
    const uint32_t depthStencilState = 1 << 4;
    const uint32_t rasterizer = 1 << 5;
    const uint32_t blendState = 1 << 6;
    const uint32_t textures = 1 << 7;
    const uint32_t constants = 1 << 8;
    const uint32_t vertexBuffer = 1 << 9;
    const uint32_t indexBuffer = 1 << 10;
  }

  struct D3D9ShaderConstants {
    std::array<Vector<float, 4>, 256> floatConstants;
    std::array<Vector<int, 4>, 16> intConstants;
    std::array<int, 16> boolConstants;
  };

  class ID3D9State {
  public:
    virtual HRESULT SetTexture(DWORD sampler, IDirect3DBaseTexture9* texture) = 0;
    virtual HRESULT GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) = 0;

    virtual HRESULT GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) = 0;
    virtual HRESULT SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) = 0;

    virtual HRESULT GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) = 0;
    virtual HRESULT SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) = 0;

    virtual HRESULT GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) = 0;
    virtual HRESULT SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) = 0;

    virtual HRESULT GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) = 0;
    virtual HRESULT SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) = 0;

    virtual HRESULT GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) = 0;
    virtual HRESULT SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) = 0;

    virtual HRESULT GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) = 0;
    virtual HRESULT SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) = 0;

    virtual HRESULT GetVertexShader(IDirect3DVertexShader9** ppShader) = 0;
    virtual HRESULT SetVertexShader(IDirect3DVertexShader9* pShader) = 0;

    virtual HRESULT GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) = 0;
    virtual HRESULT SetVertexShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount) = 0;

    virtual HRESULT GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) = 0;
    virtual HRESULT SetVertexShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount) = 0;

    virtual HRESULT GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) = 0;
    virtual HRESULT SetVertexShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount) = 0;

    virtual HRESULT GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) = 0;
    virtual HRESULT SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) = 0;

    virtual HRESULT GetIndices(IDirect3DIndexBuffer9** ppIndexData) = 0;
    virtual HRESULT SetIndices(IDirect3DIndexBuffer9* pIndexData) = 0;

    virtual HRESULT GetPixelShader(IDirect3DPixelShader9** ppShader) = 0;
    virtual HRESULT SetPixelShader(IDirect3DPixelShader9* pShader) = 0;

    virtual HRESULT GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) = 0;
    virtual HRESULT SetPixelShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount) = 0;

    virtual HRESULT GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) = 0;
    virtual HRESULT SetPixelShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount) = 0;

    virtual HRESULT GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) = 0;
    virtual HRESULT SetPixelShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount) = 0;
  };

  class D3D9State : public ID3D9State {
  public:
    D3D9State();

    HRESULT GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) override;
    HRESULT SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) override;

    HRESULT GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) override;
    HRESULT SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) override;

    HRESULT GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) override;
    HRESULT SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) override;

    HRESULT GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) override;
    HRESULT SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) override;

    HRESULT GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) override;
    HRESULT SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) override;

    HRESULT GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) override;
    HRESULT SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) override;

    HRESULT GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) override;
    HRESULT SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) override;

    HRESULT GetVertexShader(IDirect3DVertexShader9** ppShader) override;
    HRESULT SetVertexShader(IDirect3DVertexShader9* pShader) override;

    HRESULT GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) override;
    HRESULT SetVertexShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount) override;

    HRESULT GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) override;
    HRESULT SetVertexShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount) override;

    HRESULT GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) override;
    HRESULT SetVertexShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount) override;

    HRESULT GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) override;
    HRESULT SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) override;

    HRESULT GetIndices(IDirect3DIndexBuffer9** ppIndexData) override;
    HRESULT SetIndices(IDirect3DIndexBuffer9* pIndexData) override;

    HRESULT GetPixelShader(IDirect3DPixelShader9** ppShader) override;
    HRESULT SetPixelShader(IDirect3DPixelShader9* pShader) override;

    HRESULT GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) override;
    HRESULT SetPixelShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount) override;

    HRESULT GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) override;
    HRESULT SetPixelShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount) override;

    HRESULT GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) override;
    HRESULT SetPixelShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount) override;

  protected:

    // Coming soon :)
    //friend class D3D9Renderer;
    friend class Direct3DDevice9Ex;

    uint32_t dirtyFlags;
    uint32_t dirtySamplers;

    // Manual COM
    std::array<IDirect3DBaseTexture9*, 20> textures;

    ComPrivate<Direct3DVertexShader9> vertexShader;
    ComPrivate<Direct3DPixelShader9> pixelShader;
    ComPrivate<Direct3DVertexDeclaration9> vertexDecl;
    ComPrivate<Direct3DSurface9> depthStencil;
    std::array<ComPrivate<Direct3DSurface9>, 4> renderTargets;

    std::array<ComPrivate<Direct3DVertexBuffer9>, 16> vertexBuffers;

    std::array<UINT, 16> vertexOffsets;
    std::array<UINT, 16> vertexStrides;

    std::array<DWORD, D3DRS_BLENDOPALPHA + 1> renderState;
    std::array<std::array<DWORD, D3DSAMP_DMAPOFFSET + 1>, 20> samplerStates;
    std::array<std::array<DWORD, D3DTSS_CONSTANT + 1>, 8> textureStageStates;

    ComPrivate<Direct3DIndexBuffer9> indexBuffer;

    D3D9ShaderConstants vsConstants;
    D3D9ShaderConstants psConstants;
  };

}