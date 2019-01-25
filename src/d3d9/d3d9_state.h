#pragma once

#include "d3d9_base.h"
#include "d3d9_device.h"
#include "d3d9_device_unknown.h"
#include "d3d9_shaders.h"
#include "d3d9_vertexdeclaration.h"
#include "d3d9_buffer.h"
#include "d3d9_constant_buffer.h"

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
    const uint32_t vsConstants = 1 << 8;
    const uint32_t psConstants = 1 << 9;
    const uint32_t vertexBuffers = 1 << 10;
    const uint32_t indexBuffer = 1 << 11;
    const uint32_t scissorRect = 1 << 12;
    const uint32_t viewport = 1 << 13;
  }

  class D3D9State {
  public:
    D3D9State(Direct3DDevice9Ex* device, uint32_t stateBlockType);

    HRESULT GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture);
    HRESULT SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture);

    HRESULT GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget);
    HRESULT SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);

    HRESULT GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface);
    HRESULT SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil);

    HRESULT GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue);
    HRESULT SetRenderState(D3DRENDERSTATETYPE State, DWORD Value);

    HRESULT GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue);
    HRESULT SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);

    HRESULT GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue);
    HRESULT SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);

    HRESULT GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl);
    HRESULT SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl);

    HRESULT GetVertexShader(IDirect3DVertexShader9** ppShader);
    HRESULT SetVertexShader(IDirect3DVertexShader9* pShader);

    HRESULT GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount);
    HRESULT SetVertexShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount);

    HRESULT GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount);
    HRESULT SetVertexShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount);

    HRESULT GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount);
    HRESULT SetVertexShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount);

    HRESULT GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride);
    HRESULT SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride);

    HRESULT GetIndices(IDirect3DIndexBuffer9** ppIndexData);
    HRESULT SetIndices(IDirect3DIndexBuffer9* pIndexData);

    HRESULT GetPixelShader(IDirect3DPixelShader9** ppShader);
    HRESULT SetPixelShader(IDirect3DPixelShader9* pShader);

    HRESULT GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount);
    HRESULT SetPixelShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount);

    HRESULT GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount);
    HRESULT SetPixelShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount);

    HRESULT GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount);
    HRESULT SetPixelShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount);

    HRESULT GetScissorRect(RECT* pRect);
    HRESULT SetScissorRect(CONST RECT* pRect);

    HRESULT GetViewport(D3DVIEWPORT9* pViewport);
    HRESULT SetViewport(CONST D3DVIEWPORT9* pViewport);

    void capture(uint32_t stateBlockType, bool recapture);
    void apply();

  protected:

    friend class D3D9ImmediateRenderer;

    uint32_t dirtyFlags;
    uint32_t dirtySamplers;

    // Replace this with bitvec sometime...
    std::array<bool, D3DRS_BLENDOPALPHA + 1> textureCaptured;
    // Manual COM
    std::array<IDirect3DBaseTexture9*, 20> textures;

    bool vertexShaderCaptured = false;
    ComPrivate<Direct3DVertexShader9> vertexShader;

    bool pixelShaderCaptured = false;
    ComPrivate<Direct3DPixelShader9> pixelShader;

    bool vertexDeclCaptured = false;
    ComPrivate<Direct3DVertexDeclaration9> vertexDecl;

    ComPrivate<Direct3DSurface9> depthStencil;

    std::array<ComPrivate<Direct3DSurface9>, 4> renderTargets;

    std::array<bool, 16> vertexBufferCaptures;
    std::array<ComPrivate<Direct3DVertexBuffer9>, 16> vertexBuffers;
    std::array<UINT, 16> vertexOffsets;
    std::array<UINT, 16> vertexStrides;

    std::array<bool, D3DRS_BLENDOPALPHA + 1> renderStateCaptures;
    std::array<DWORD, D3DRS_BLENDOPALPHA + 1> renderState;

    std::array<std::array<bool, D3DSAMP_DMAPOFFSET + 1>, 20> samplerStateCaptures;
    std::array<std::array<DWORD, D3DSAMP_DMAPOFFSET + 1>, 20> samplerStates;

    std::array<std::array<bool, D3DTSS_CONSTANT + 1>, 8> textureStageStateCaptures;
    std::array<std::array<DWORD, D3DTSS_CONSTANT + 1>, 8> textureStageStates;

    bool indexBufferCaptured = false;
    ComPrivate<Direct3DIndexBuffer9> indexBuffer;

    D3D9ShaderConstants vsConstants;
    D3D9ShaderConstants psConstants;

    D3DVIEWPORT9 viewport;
    bool viewportCaptured = false;

    RECT scissorRect;
    bool scissorRectCaptured = false;

    void captureRenderState(D3DRENDERSTATETYPE state, bool recapture = false);
    void captureTextureStageState(uint32_t stage, D3DTEXTURESTAGESTATETYPE type, bool recapture = false);
    void captureSamplerState(uint32_t sampler, D3DSAMPLERSTATETYPE type, bool recapture = false);

    void capturePixelRenderStates(bool recapture = false);
    void capturePixelTextureStates(bool recapture = false);
    void capturePixelSamplerStates(bool recapture = false);
    void capturePixelShaderStates(bool recapture = false);
    void captureVertexRenderStates(bool recapture = false);
    void captureVertexSamplerStates(bool recapture = false);
    void captureVertexTextureStates(bool recapture = false);
    void captureVertexShaderStates(bool recapture = false);
    void captureVertexDeclaration(bool recapture = false);
    void captureTextures(bool recapture = false);
    void captureVertexStreams(bool recapture = false);
    void captureIndexBuffer(bool recapture = false);
    void captureViewport(bool recapture = false);
    void captureScissor(bool recapture = false);

    Direct3DDevice9Ex* m_device;
  };

  class Direct3DStateBlock9 : public D3D9DeviceUnknown<IDirect3DStateBlock9> {
  public:

    Direct3DStateBlock9(Direct3DDevice9Ex* device, uint32_t stateBlockType);

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv) override;
    HRESULT STDMETHODCALLTYPE Capture() override;
    HRESULT STDMETHODCALLTYPE Apply() override;

    inline D3D9State* GetState() {
      return &m_state;
    }

  private:

    D3D9State m_state;
  };

}