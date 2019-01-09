#include "d3d9_state.h"
#include "d3d9_util.h"
#include "d3d9_texture.h"

namespace dxup {

  D3D9State::D3D9State() 
    : dirtyFlags{ 0 }, dirtySamplers{ 0 } {
    std::memset(textures.data(), 0, sizeof(IDirect3DBaseTexture9*) * textures.size());
    std::memset(vertexOffsets.data(), 0, sizeof(UINT) * vertexOffsets.size());
    std::memset(vertexStrides.data(), 0, sizeof(UINT) * vertexStrides.size());
  }

  HRESULT D3D9State::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
    InitReturnPtr(ppTexture);

    if (ppTexture == nullptr)
      return D3DERR_INVALIDCALL;

    if (FAILED(convert::mapStageToSampler(Stage, &Stage)))
      return D3DERR_INVALIDCALL;

    *ppTexture = ref(textures[Stage]);

    return D3D_OK;
  }
  HRESULT D3D9State::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
    if (FAILED(convert::mapStageToSampler(Stage, &Stage)))
      return D3DERR_INVALIDCALL;

    if (textures[Stage] == pTexture)
      return D3D_OK;

    IDirect3DBaseTexture9* currentBinding = textures[Stage];

    if (currentBinding != nullptr) {
      switch (textures[Stage]->GetType()) {
      case D3DRTYPE_TEXTURE: reinterpret_cast<Direct3DTexture9*>(currentBinding)->ReleasePrivate(); break;
      case D3DRTYPE_CUBETEXTURE: reinterpret_cast<Direct3DCubeTexture9*>(currentBinding)->ReleasePrivate(); break;
      default:
        log::warn("Unable to find what texture stage really is to release internally.");
        break;
      }
    }

    if (pTexture != nullptr) {
      switch (pTexture->GetType()) {

      case D3DRTYPE_TEXTURE: {
        Direct3DTexture9* tex = reinterpret_cast<Direct3DTexture9*>(pTexture);
        tex->AddRefPrivate();
        break;
      }

      case D3DRTYPE_CUBETEXTURE: {
        Direct3DCubeTexture9* tex = reinterpret_cast<Direct3DCubeTexture9*>(pTexture);
        tex->AddRefPrivate();
        break;
      }

      default: {
        textures[Stage] = nullptr;
        log::warn("Unable to find what new texture to bind really is.");
        break;
      }

      }
    }

    textures[Stage] = pTexture;
    dirtyFlags |= dirtyFlags::textures;
    return D3D_OK;
  }

  HRESULT D3D9State::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
    InitReturnPtr(ppRenderTarget);

    if (ppRenderTarget == nullptr)
      return D3DERR_INVALIDCALL;

    if (RenderTargetIndex > renderTargets.size())
      return D3DERR_INVALIDCALL;

    if (renderTargets[RenderTargetIndex] == nullptr)
      return D3DERR_NOTFOUND;

    *ppRenderTarget = (IDirect3DSurface9*) ref(renderTargets[RenderTargetIndex]);

    return D3D_OK;
  }
  HRESULT D3D9State::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
    if (RenderTargetIndex >= 4)
      return D3DERR_INVALIDCALL;

    renderTargets[RenderTargetIndex] = reinterpret_cast<Direct3DSurface9*>(pRenderTarget);
    dirtyFlags |= dirtyFlags::renderTargets;

    return D3D_OK;
  }

  HRESULT D3D9State::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
    InitReturnPtr(ppZStencilSurface);

    if (ppZStencilSurface == nullptr)
      return D3DERR_INVALIDCALL;

    if (depthStencil == nullptr)
      return D3DERR_NOTFOUND;

    *ppZStencilSurface = (IDirect3DSurface9*) ref(depthStencil);

    return D3D_OK;
  }
  HRESULT D3D9State::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {
    Direct3DSurface9* newSurface = reinterpret_cast<Direct3DSurface9*>(pNewZStencil);
    if (depthStencil == newSurface)
      return D3D_OK;

    depthStencil = newSurface;
    dirtyFlags |= dirtyFlags::renderTargets;

    return D3D_OK;
  }

  HRESULT D3D9State::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {
    if (pValue == nullptr)
      return D3DERR_INVALIDCALL;

    if (State < D3DRS_ZENABLE || State > D3DRS_BLENDOPALPHA) {
      *pValue = 0;

      return D3D_OK;
    }

    *pValue = renderState[State];

    return D3D_OK;
  }
  HRESULT D3D9State::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {

    if (State < D3DRS_ZENABLE || State > D3DRS_BLENDOPALPHA)
      return D3D_OK;

    if (renderState[State] == Value)
      return D3D_OK;

    renderState[State] = Value;

    if (State == D3DRS_CULLMODE ||
      State == D3DRS_DEPTHBIAS ||
      State == D3DRS_FILLMODE ||
      State == D3DRS_SCISSORTESTENABLE ||
      State == D3DRS_SLOPESCALEDEPTHBIAS)
      dirtyFlags |= dirtyFlags::rasterizer;
    else if (State == D3DRS_CCW_STENCILZFAIL ||
      State == D3DRS_CCW_STENCILFAIL ||
      State == D3DRS_CCW_STENCILPASS ||
      State == D3DRS_CCW_STENCILFUNC ||

      State == D3DRS_ZENABLE ||
      State == D3DRS_ZFUNC ||
      State == D3DRS_ZWRITEENABLE ||

      State == D3DRS_STENCILZFAIL ||
      State == D3DRS_STENCILFAIL ||
      State == D3DRS_STENCILPASS ||
      State == D3DRS_STENCILFUNC ||

      State == D3DRS_STENCILENABLE ||
      State == D3DRS_STENCILMASK ||
      State == D3DRS_STENCILWRITEMASK ||

      State == D3DRS_STENCILREF)
      dirtyFlags |= dirtyFlags::depthStencilState;
    else if (State == D3DRS_BLENDOP ||
      State == D3DRS_BLENDOPALPHA ||
      State == D3DRS_DESTBLEND ||
      State == D3DRS_DESTBLENDALPHA ||
      State == D3DRS_SRCBLEND ||
      State == D3DRS_SRCBLENDALPHA ||
      State == D3DRS_SEPARATEALPHABLENDENABLE ||
      State == D3DRS_ALPHABLENDENABLE ||
      State == D3DRS_BLENDFACTOR ||
      State == D3DRS_COLORWRITEENABLE ||
      State == D3DRS_COLORWRITEENABLE1 ||
      State == D3DRS_COLORWRITEENABLE2 ||
      State == D3DRS_COLORWRITEENABLE3)
      dirtyFlags |= dirtyFlags::blendState;
    else if (State == D3DRS_SRGBWRITEENABLE)
      dirtyFlags |= dirtyFlags::renderTargets;
    else
      log::warn("Unhandled render state: %lu", State);

    return D3D_OK;
  }

  HRESULT D3D9State::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
    if (pValue == nullptr)
      return D3DERR_INVALIDCALL;

    if (Type < D3DTSS_COLOROP || Type > D3DTSS_CONSTANT)
      return D3D_OK;

    if (Stage > 7)
      return D3D_OK;

    *pValue = textureStageStates[Stage][Type];

    return D3D_OK;
  }
  HRESULT D3D9State::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
    if (Type < D3DTSS_COLOROP || Type > D3DTSS_CONSTANT)
      return D3D_OK;

    if (Stage > 7)
      return D3D_OK;

    if (textureStageStates[Stage][Type] == Value)
      return D3D_OK;

    textureStageStates[Stage][Type] = Value;
    //dirtyTextureStage |= 1 << Stage;

    return D3D_OK;
  }

  HRESULT D3D9State::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
    if (pValue == nullptr)
      return D3DERR_INVALIDCALL;

    if (FAILED(convert::mapStageToSampler(Sampler, &Sampler)))
      return D3DERR_INVALIDCALL;

    if (Type < D3DSAMP_ADDRESSU || Type > D3DSAMP_DMAPOFFSET)
      return D3D_OK;

    *pValue = samplerStates[Sampler][Type];

    return D3D_OK;
  }
  HRESULT D3D9State::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
    if (FAILED(convert::mapStageToSampler(Sampler, &Sampler)))
      return D3DERR_INVALIDCALL;

    if (Type < D3DSAMP_ADDRESSU || Type > D3DSAMP_DMAPOFFSET)
      return D3D_OK;

    if (samplerStates[Sampler][Type] == Value)
      return D3D_OK;

    samplerStates[Sampler][Type] = Value;
    dirtySamplers |= 1 << Sampler;

    return D3D_OK;
  }

  HRESULT D3D9State::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {
    Direct3DVertexDeclaration9* newDecl = reinterpret_cast<Direct3DVertexDeclaration9*>(pDecl);

    if (vertexDecl == newDecl)
      return D3D_OK;

    vertexDecl = newDecl;
    dirtyFlags |= dirtyFlags::vertexDecl;

    return D3D_OK;
  }
  HRESULT D3D9State::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {
    InitReturnPtr(ppDecl);

    if (ppDecl == nullptr)
      return D3DERR_INVALIDCALL;

    if (vertexDecl == nullptr)
      return D3DERR_NOTFOUND;

    *ppDecl = ref(vertexDecl);

    return D3D_OK;
  }

  HRESULT D3D9State::GetVertexShader(IDirect3DVertexShader9** ppShader) {
    InitReturnPtr(ppShader);

    if (ppShader == nullptr)
      return D3DERR_INVALIDCALL;

    if (vertexShader == nullptr)
      return D3DERR_NOTFOUND;

    *ppShader = ref(vertexShader);

    return D3D_OK;
  }
  HRESULT D3D9State::SetVertexShader(IDirect3DVertexShader9* pShader) {
    dirtyFlags |= dirtyFlags::vertexShader;

    if (pShader == nullptr) {
      vertexShader = nullptr;
      return D3D_OK;
    }

    vertexShader = reinterpret_cast<Direct3DVertexShader9*>(pShader);

    return D3D_OK;
  }

  HRESULT D3D9State::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
    if (pConstantData == nullptr)
      return D3DERR_INVALIDCALL;

    if (Vector4fCount == 0)
      return D3D_OK;

    arrayCopyJ(pConstantData, &vsConstants.floatConstants[StartRegister], Vector4fCount);
    return D3D_OK;
  }
  HRESULT D3D9State::SetVertexShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount) {
    if (pConstantData == nullptr)
      return D3DERR_INVALIDCALL;

    if (Vector4fCount == 0)
      return D3D_OK;

    dirtyFlags |= dirtyFlags::vsConstants;

    arrayCopyT(&vsConstants.floatConstants[StartRegister], pConstantData, Vector4fCount);
    return D3D_OK;
  }

  HRESULT D3D9State::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
    if (pConstantData == nullptr)
      return D3DERR_INVALIDCALL;

    if (Vector4iCount == 0)
      return D3D_OK;

    arrayCopyJ(pConstantData, &vsConstants.intConstants[StartRegister], Vector4iCount);
    return D3D_OK;
  }
  HRESULT D3D9State::SetVertexShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount) {
    if (pConstantData == nullptr)
      return D3DERR_INVALIDCALL;

    if (Vector4iCount == 0)
      return D3D_OK;

    dirtyFlags |= dirtyFlags::vsConstants;
    arrayCopyT(&vsConstants.intConstants[StartRegister], pConstantData, Vector4iCount);
    return D3D_OK;
  }

  HRESULT D3D9State::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
    if (pConstantData == nullptr)
      return D3DERR_INVALIDCALL;

    if (BoolCount == 0)
      return D3D_OK;

    arrayCopyJ(pConstantData, &vsConstants.boolConstants[StartRegister], BoolCount);
    return D3D_OK;
  }
  HRESULT D3D9State::SetVertexShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount) {
    if (pConstantData == nullptr)
      return D3DERR_INVALIDCALL;

    if (BoolCount == 0)
      return D3D_OK;

    dirtyFlags |= dirtyFlags::vsConstants;
    arrayCopyT(&vsConstants.boolConstants[StartRegister], pConstantData, BoolCount);
    return D3D_OK;
  }

  HRESULT D3D9State::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {
    if (StreamNumber >= 16 || ppStreamData == nullptr || pOffsetInBytes == nullptr)
      return D3DERR_INVALIDCALL;

    *pOffsetInBytes = 0;

    if (pStride == nullptr)
      return D3DERR_INVALIDCALL;

    *pStride = 0;

    if (vertexBuffers[StreamNumber] == nullptr)
      return D3DERR_NOTFOUND;

    *ppStreamData = ref(vertexBuffers[StreamNumber]);
    *pOffsetInBytes = vertexOffsets[StreamNumber];
    *pStride = vertexStrides[StreamNumber];

    return D3D_OK;
  }
  HRESULT D3D9State::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {
    if (StreamNumber >= 16)
      return D3DERR_INVALIDCALL;

    Direct3DVertexBuffer9* vertexBuffer = reinterpret_cast<Direct3DVertexBuffer9*>(pStreamData);

    ID3D11Buffer* buffer = nullptr;

    if (vertexBuffer != nullptr)
      buffer = vertexBuffer->GetDXUPResource()->GetResourceAs<ID3D11Buffer>();

    vertexBuffers[StreamNumber] = vertexBuffer;
    vertexOffsets[StreamNumber] = OffsetInBytes;
    vertexStrides[StreamNumber] = Stride;

    dirtyFlags |= dirtyFlags::vertexBuffers;

    return D3D_OK;
  }

  HRESULT D3D9State::GetIndices(IDirect3DIndexBuffer9** ppIndexData) {
    InitReturnPtr(ppIndexData);

    if (ppIndexData == nullptr)
      return D3DERR_INVALIDCALL;

    if (indexBuffer == nullptr)
      return D3DERR_NOTFOUND;

    *ppIndexData = ref(indexBuffer);

    return D3D_OK;
  }
  HRESULT D3D9State::SetIndices(IDirect3DIndexBuffer9* pIndexData) {
    Direct3DIndexBuffer9* indices = reinterpret_cast<Direct3DIndexBuffer9*>(pIndexData);

    if (indexBuffer == indices)
      return D3D_OK;

    indexBuffer = indices;
    dirtyFlags |= dirtyFlags::indexBuffer;

    return D3D_OK;
  }

  HRESULT D3D9State::GetPixelShader(IDirect3DPixelShader9** ppShader) {
    InitReturnPtr(ppShader);

    if (ppShader == nullptr)
      return D3DERR_INVALIDCALL;

    if (pixelShader == nullptr)
      return D3DERR_NOTFOUND;

    *ppShader = ref(pixelShader);

    return D3D_OK;
  }
  HRESULT D3D9State::SetPixelShader(IDirect3DPixelShader9* pShader) {
    dirtyFlags |= dirtyFlags::pixelShader;

    if (pShader == nullptr) {
      pixelShader = nullptr;
      return D3D_OK;
    }

    pixelShader = reinterpret_cast<Direct3DPixelShader9*>(pShader);

    return D3D_OK;
  }

  HRESULT D3D9State::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
    if (pConstantData == nullptr)
      return D3DERR_INVALIDCALL;

    if (Vector4fCount == 0)
      return D3D_OK;

    arrayCopyJ(pConstantData, &psConstants.floatConstants[StartRegister], Vector4fCount);
    return D3D_OK;
  }
  HRESULT D3D9State::SetPixelShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount) {
    if (pConstantData == nullptr)
      return D3DERR_INVALIDCALL;

    if (Vector4fCount == 0)
      return D3D_OK;

    dirtyFlags |= dirtyFlags::psConstants;
    arrayCopyT(&psConstants.floatConstants[StartRegister], pConstantData, Vector4fCount);
    return D3D_OK;
  }

  HRESULT D3D9State::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
    if (pConstantData == nullptr)
      return D3DERR_INVALIDCALL;

    if (Vector4iCount == 0)
      return D3D_OK;

    arrayCopyJ(pConstantData, &psConstants.intConstants[StartRegister], Vector4iCount);
    return D3D_OK;
  }
  HRESULT D3D9State::SetPixelShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount) {
    if (pConstantData == nullptr)
      return D3DERR_INVALIDCALL;

    if (Vector4iCount == 0)
      return D3D_OK;

    dirtyFlags |= dirtyFlags::psConstants;
    arrayCopyT(&psConstants.intConstants[StartRegister], pConstantData, Vector4iCount);
    return D3D_OK;
  }

  HRESULT D3D9State::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
    if (pConstantData == nullptr)
      return D3DERR_INVALIDCALL;

    if (BoolCount == 0)
      return D3D_OK;

    arrayCopyJ(pConstantData, &psConstants.boolConstants[StartRegister], BoolCount);
    return D3D_OK;
  }
  HRESULT D3D9State::SetPixelShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount) {
    if (pConstantData == nullptr)
      return D3DERR_INVALIDCALL;

    if (BoolCount == 0)
      return D3D_OK;

    dirtyFlags |= dirtyFlags::psConstants;
    arrayCopyT(&psConstants.boolConstants[StartRegister], pConstantData, BoolCount);
    return D3D_OK;
  }
}