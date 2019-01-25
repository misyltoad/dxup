#include "d3d9_state.h"
#include "d3d9_util.h"
#include "d3d9_texture.h"

namespace dxup {

  // I hate this whole file. It sucks.
  // I really want to clean up state capturing sometime but I'm not really sure how.
  // I thought about having a generic state class, but something like that wouldn't really work
  // nicely for arrays of constants that like to be memcpy'ed or accessed efficiently.
  // Feel free to have a stab at cleaning this mess up.
  // I am sorry. I feel like I let everyone down taking so long to make this piece of crap.

  D3D9State::D3D9State(Direct3DDevice9Ex* device, uint32_t stateBlockType)
    : m_device{ device } {
    dirtyFlags = 0;
    dirtySamplers = 0;

    std::memset(textures.data(), 0, sizeof(IDirect3DBaseTexture9*) * textures.size());
    std::memset(vertexOffsets.data(), 0, sizeof(UINT) * vertexOffsets.size());
    std::memset(vertexStrides.data(), 0, sizeof(UINT) * vertexStrides.size());

    if (stateBlockType != 0)
      this->capture(stateBlockType, false);
  }

  void D3D9State::capture(uint32_t stateBlockType, bool recapture) {
    // Using 0x80000000 to denote uncaptured as it'll never be a value for any of our constant types. (-0.0 fl)
    // Idea taken from SwiftShader. May or may not be accurate.
    for (uint32_t i = 0; i < 2; i++) {
      D3D9ShaderConstants& constants = i == 0 ? vsConstants : psConstants;
      std::memset(constants.floatConstants.data(), 0x80000000, sizeof(constants.floatConstants));
      std::memset(constants.intConstants.data(), 0x80000000, sizeof(constants.intConstants));
      std::memset(constants.boolConstants.data(), 0x80000000, sizeof(constants.boolConstants));
    }

    if (stateBlockType == D3DSBT_PIXELSTATE || stateBlockType == D3DSBT_ALL)
    {
      this->capturePixelRenderStates(recapture);
      this->capturePixelTextureStates(recapture);
      this->capturePixelSamplerStates(recapture);
      this->capturePixelShaderStates(recapture);
    }
    if (stateBlockType == D3DSBT_VERTEXSTATE || stateBlockType == D3DSBT_ALL)
    {
      this->captureVertexRenderStates(recapture);
      this->captureVertexSamplerStates(recapture);
      this->captureVertexTextureStates(recapture);
      this->captureVertexShaderStates(recapture);
      this->captureVertexDeclaration(recapture);
    }
    if (stateBlockType == D3DSBT_ALL)   // Capture remaining states
    {
      this->captureTextures(recapture);
      this->captureVertexStreams(recapture);
      this->captureIndexBuffer(recapture);
      this->captureViewport(recapture);
      this->captureScissor(recapture);
      //captureClipPlanes(recapture);

      // There is more crap here too!
    }
  }
  void D3D9State::apply() {
    if (viewportCaptured)
      m_device->SetViewport(&viewport);

    if (scissorRectCaptured)
      m_device->SetScissorRect(&scissorRect);

    if (vertexShaderCaptured)
      m_device->SetVertexShader(vertexShader.ptr());

    if (pixelShaderCaptured)
      m_device->SetPixelShader(pixelShader.ptr());

    if (vertexDeclCaptured)
      m_device->SetVertexDeclaration(vertexDecl.ptr());

    for (uint32_t i = 0; i < 16; i++) {
      if (vertexBufferCaptures[i])
        m_device->SetStreamSource(i, vertexBuffers[i].ptr(), vertexOffsets[i], vertexStrides[i]);
    }

    for (uint32_t i = 0; i < renderState.size(); i++) {
      if (renderStateCaptures[i])
        m_device->SetRenderState((D3DRENDERSTATETYPE)i, renderState[i]);
    }

    forEachSampler([&](uint32_t i) {
      for (uint32_t j = 0; j < D3DSAMP_DMAPOFFSET + 1; j++) {
        uint32_t internalSampler = 0;
        convert::mapStageToSampler(i, &internalSampler);

        if (samplerStateCaptures[internalSampler][j])
          m_device->SetSamplerState(i, (D3DSAMPLERSTATETYPE)j, renderState[j]);
      }
    });

    for (uint32_t i = 0; i < textureStageStates.size(); i++) {
      for (uint32_t j = 0; j < D3DTSS_CONSTANT + 1; j++) {
        if (textureStageStateCaptures[i][j])
          m_device->SetTextureStageState(i, (D3DTEXTURESTAGESTATETYPE)j, textureStageStates[i][j]);
      }
    }

    if (indexBufferCaptured)
      m_device->SetIndices(indexBuffer.ptr());

    // TODO consolidate this into one code path...
    // VS

    for (uint32_t i = 0; i < vsConstants.floatConstants.size(); i++) {
      uint32_t uintData = *((uint32_t*)(&vsConstants.floatConstants[i].data[0]));
      if (uintData != 0x80000000u)
       m_device->SetVertexShaderConstantF(i, (float*)&vsConstants.floatConstants[i], 1);
    }

    for (uint32_t i = 0; i < vsConstants.intConstants.size(); i++) {
      if ((UINT)vsConstants.intConstants[i].data[0] != 0x80000000u)
        m_device->SetVertexShaderConstantI(i, (int*)&vsConstants.intConstants[i], 1);
    }

    for (uint32_t i = 0; i < vsConstants.boolConstants.size(); i++) {
      if ((UINT)vsConstants.boolConstants[i] != 0x80000000u)
        m_device->SetVertexShaderConstantB(i, (int*)&vsConstants.boolConstants[i], 1);
    }
    
    // PS

    for (uint32_t i = 0; i < psConstants.floatConstants.size(); i++) {
      uint32_t uintData = *((uint32_t*)(&psConstants.floatConstants[i].data[0]));
      if (uintData != 0x80000000)
        m_device->SetPixelShaderConstantF(i, (float*)&psConstants.floatConstants[i], 1);
    }

    for (uint32_t i = 0; i < psConstants.intConstants.size(); i++) {
      if ((UINT)psConstants.intConstants[i].data[0] != 0x80000000u)
        m_device->SetPixelShaderConstantI(i, (int*)&psConstants.intConstants[i], 1);
    }

    for (uint32_t i = 0; i < psConstants.boolConstants.size(); i++) {
      if ((UINT)psConstants.boolConstants[i] != 0x80000000u)
        m_device->SetPixelShaderConstantB(i, (int*)&psConstants.boolConstants[i], 1);
    }

    // viewport, scissor, clip planes, etc...
  }

  HRESULT D3D9State::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
    InitReturnPtr(ppTexture);

    if (ppTexture == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetTexture: ppTexture was nullptr.");

    HRESULT result = convert::mapStageToSampler(Stage, &Stage);
    if (FAILED(result))
      return result;

    *ppTexture = ref(textures[Stage]);

    return D3D_OK;
  }
  HRESULT D3D9State::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
    HRESULT result = convert::mapStageToSampler(Stage, &Stage);
    if (FAILED(result))
      return result;

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

    textureCaptured[Stage] = true;
    textures[Stage] = pTexture;
    dirtyFlags |= dirtyFlags::textures;
    return D3D_OK;
  }

  HRESULT D3D9State::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
    InitReturnPtr(ppRenderTarget);

    if (ppRenderTarget == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetRenderTarget: ppRenderTarget was nullptr.");

    if (RenderTargetIndex > renderTargets.size())
      return log::d3derr(D3DERR_INVALIDCALL, "GetRenderTarget: rendertarget index out of bounds (%d).", RenderTargetIndex);

    if (renderTargets[RenderTargetIndex] == nullptr)
      return D3DERR_NOTFOUND;

    *ppRenderTarget = (IDirect3DSurface9*) ref(renderTargets[RenderTargetIndex]);

    return D3D_OK;
  }
  HRESULT D3D9State::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
    if (RenderTargetIndex >= 4)
      return log::d3derr(D3DERR_INVALIDCALL, "SetRenderTarget: rendertarget index out of bounds (%d).", RenderTargetIndex);

    renderTargets[RenderTargetIndex] = reinterpret_cast<Direct3DSurface9*>(pRenderTarget);

    if (renderTargets[RenderTargetIndex] != nullptr && RenderTargetIndex == 0) {
      D3DSURFACE_DESC desc;
      renderTargets[RenderTargetIndex]->GetDesc(&desc);

      viewport.X = 0;
      viewport.Y = 0;
      viewport.Width = desc.Width;
      viewport.Height = desc.Height;
      viewport.MinZ = 0;
      viewport.MaxZ = 1;
      dirtyFlags |= dirtyFlags::viewport;

      scissorRect.left = 0;
      scissorRect.top = 0;
      scissorRect.right = desc.Width;
      scissorRect.bottom = desc.Height;
      dirtyFlags |= dirtyFlags::scissorRect;
    }

    dirtyFlags |= dirtyFlags::renderTargets;

    return D3D_OK;
  }

  HRESULT D3D9State::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
    InitReturnPtr(ppZStencilSurface);

    if (ppZStencilSurface == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetDepthStencilSurface: ppZStencilSurface was nullptr.");

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
      return log::d3derr(D3DERR_INVALIDCALL, "GetRenderState: pValue was nullptr.");

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

    renderStateCaptures[State] = true;

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
      return log::d3derr(D3DERR_INVALIDCALL, "GetTextureStageState: pValue was nullptr.");

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

    textureStageStateCaptures[Stage][Type] = true;

    if (textureStageStates[Stage][Type] == Value)
      return D3D_OK;

    textureStageStates[Stage][Type] = Value;
    //dirtyTextureStage |= 1 << Stage;

    return D3D_OK;
  }

  HRESULT D3D9State::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
    if (pValue == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetSamplerState: pValue was nullptr.");

    HRESULT result = convert::mapStageToSampler(Sampler, &Sampler);
    if (FAILED(result))
      return result;

    if (Type < D3DSAMP_ADDRESSU || Type > D3DSAMP_DMAPOFFSET)
      return D3D_OK;

    *pValue = samplerStates[Sampler][Type];

    return D3D_OK;
  }
  HRESULT D3D9State::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
    HRESULT result = convert::mapStageToSampler(Sampler, &Sampler);
    if (FAILED(result))
      return D3DERR_INVALIDCALL;

    if (Type < D3DSAMP_ADDRESSU || Type > D3DSAMP_DMAPOFFSET)
      return D3D_OK;

    samplerStateCaptures[Sampler][Type] = true;

    if (samplerStates[Sampler][Type] == Value)
      return D3D_OK;

    samplerStates[Sampler][Type] = Value;
    dirtySamplers |= 1 << Sampler;

    return D3D_OK;
  }

  HRESULT D3D9State::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {
    Direct3DVertexDeclaration9* newDecl = reinterpret_cast<Direct3DVertexDeclaration9*>(pDecl);

    vertexDeclCaptured = true;

    if (vertexDecl == newDecl)
      return D3D_OK;

    vertexDecl = newDecl;
    dirtyFlags |= dirtyFlags::vertexDecl;

    return D3D_OK;
  }
  HRESULT D3D9State::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {
    InitReturnPtr(ppDecl);

    if (ppDecl == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetVertexDeclaration: ppDecl was nullptr.");

    if (vertexDecl == nullptr)
      return D3DERR_NOTFOUND;

    *ppDecl = ref(vertexDecl);

    return D3D_OK;
  }

  HRESULT D3D9State::GetVertexShader(IDirect3DVertexShader9** ppShader) {
    InitReturnPtr(ppShader);

    if (ppShader == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetVertexShader: ppShader was nullptr.");

    if (vertexShader == nullptr)
      return D3DERR_NOTFOUND;

    *ppShader = ref(vertexShader);

    return D3D_OK;
  }
  HRESULT D3D9State::SetVertexShader(IDirect3DVertexShader9* pShader) {
    dirtyFlags |= dirtyFlags::vertexShader;

    vertexShaderCaptured = true;

    if (pShader == nullptr) {
      vertexShader = nullptr;
      return D3D_OK;
    }

    vertexShader = reinterpret_cast<Direct3DVertexShader9*>(pShader);

    return D3D_OK;
  }

  HRESULT D3D9State::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
    if (Vector4fCount == 0)
      return D3D_OK;
    
    if (pConstantData == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetVertexShaderConstantF: pConstantData was nullptr");

    arrayCopyJ(pConstantData, &vsConstants.floatConstants[StartRegister], Vector4fCount);
    return D3D_OK;
  }
  HRESULT D3D9State::SetVertexShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount) {
    if (Vector4fCount == 0)
      return D3D_OK;

    if (pConstantData == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "SetVertexShaderConstantF: pConstantData was nullptr");

    dirtyFlags |= dirtyFlags::vsConstants;

    arrayCopyT(&vsConstants.floatConstants[StartRegister], pConstantData, Vector4fCount);
    return D3D_OK;
  }

  HRESULT D3D9State::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
    if (Vector4iCount == 0)
      return D3D_OK;

    if (pConstantData == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetVertexShaderConstantI: pConstantData was nullptr");

    arrayCopyJ(pConstantData, &vsConstants.intConstants[StartRegister], Vector4iCount);
    return D3D_OK;
  }
  HRESULT D3D9State::SetVertexShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount) {
    if (Vector4iCount == 0)
      return D3D_OK;

    if (pConstantData == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "SetVertexShaderConstantI: pConstantData was nullptr");

    dirtyFlags |= dirtyFlags::vsConstants;
    arrayCopyT(&vsConstants.intConstants[StartRegister], pConstantData, Vector4iCount);
    return D3D_OK;
  }

  HRESULT D3D9State::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
    if (BoolCount == 0)
      return D3D_OK;

    if (pConstantData == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetVertexShaderConstantB: pConstantData was nullptr");

    arrayCopyJ(pConstantData, &vsConstants.boolConstants[StartRegister], BoolCount);
    return D3D_OK;
  }
  HRESULT D3D9State::SetVertexShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount) {
    if (BoolCount == 0)
      return D3D_OK;

    if (pConstantData == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "SetVertexShaderConstantB: pConstantData was nullptr");

    dirtyFlags |= dirtyFlags::vsConstants;
    arrayCopyT(&vsConstants.boolConstants[StartRegister], pConstantData, BoolCount);
    return D3D_OK;
  }

  HRESULT D3D9State::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {
    InitReturnPtr(ppStreamData);

    if (StreamNumber >= 16)
      return log::d3derr(D3DERR_INVALIDCALL, "GetStreamSource: stream number was out of bounds (range: 0-15, got: %d).", StreamNumber);

    if (ppStreamData == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetStreamSource: ppStreamData was nullptr.");

    if (pOffsetInBytes == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetStreamSource: pOffsetInBytes was nullptr.");

    *pOffsetInBytes = 0;

    if (pStride == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetStreamSource: pStride was nullptr.");

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
      return log::d3derr(D3DERR_INVALIDCALL, "SetStreamSource: stream number was out of bounds (range: 0-15, got: %d).", StreamNumber);

    Direct3DVertexBuffer9* vertexBuffer = reinterpret_cast<Direct3DVertexBuffer9*>(pStreamData);

    vertexBufferCaptures[StreamNumber] = true;
    vertexBuffers[StreamNumber] = vertexBuffer;
    vertexOffsets[StreamNumber] = OffsetInBytes;
    vertexStrides[StreamNumber] = Stride;

    dirtyFlags |= dirtyFlags::vertexBuffers;

    return D3D_OK;
  }

  HRESULT D3D9State::GetIndices(IDirect3DIndexBuffer9** ppIndexData) {
    InitReturnPtr(ppIndexData);

    if (ppIndexData == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetIndices: ppIndexData was nullptr.");

    if (indexBuffer == nullptr)
      return D3DERR_NOTFOUND;

    *ppIndexData = ref(indexBuffer);

    return D3D_OK;
  }
  HRESULT D3D9State::SetIndices(IDirect3DIndexBuffer9* pIndexData) {
    Direct3DIndexBuffer9* indices = reinterpret_cast<Direct3DIndexBuffer9*>(pIndexData);

    indexBufferCaptured = true;

    if (indexBuffer == indices)
      return D3D_OK;

    indexBuffer = indices;
    dirtyFlags |= dirtyFlags::indexBuffer;

    return D3D_OK;
  }

  HRESULT D3D9State::GetPixelShader(IDirect3DPixelShader9** ppShader) {
    InitReturnPtr(ppShader);

    if (ppShader == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetPixelShader: ppShader was nullptr.");

    if (pixelShader == nullptr)
      return D3DERR_NOTFOUND;

    *ppShader = ref(pixelShader);

    return D3D_OK;
  }
  HRESULT D3D9State::SetPixelShader(IDirect3DPixelShader9* pShader) {
    dirtyFlags |= dirtyFlags::pixelShader;

    pixelShaderCaptured = true;

    if (pShader == nullptr) {
      pixelShader = nullptr;
      return D3D_OK;
    }

    pixelShader = reinterpret_cast<Direct3DPixelShader9*>(pShader);

    return D3D_OK;
  }

  HRESULT D3D9State::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
    if (pConstantData == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetPixelShaderConstantF: pConstantData was nullptr");

    if (Vector4fCount == 0)
      return D3D_OK;

    arrayCopyJ(pConstantData, &psConstants.floatConstants[StartRegister], Vector4fCount);
    return D3D_OK;
  }
  HRESULT D3D9State::SetPixelShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount) {
    if (pConstantData == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "SetPixelShaderConstantF: pConstantData was nullptr");

    if (Vector4fCount == 0)
      return D3D_OK;

    dirtyFlags |= dirtyFlags::psConstants;
    arrayCopyT(&psConstants.floatConstants[StartRegister], pConstantData, Vector4fCount);
    return D3D_OK;
  }

  HRESULT D3D9State::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
    if (pConstantData == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetPixelShaderConstantI: pConstantData was nullptr");

    if (Vector4iCount == 0)
      return D3D_OK;

    arrayCopyJ(pConstantData, &psConstants.intConstants[StartRegister], Vector4iCount);
    return D3D_OK;
  }
  HRESULT D3D9State::SetPixelShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount) {
    if (pConstantData == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "SetPixelShaderConstantI: pConstantData was nullptr");

    if (Vector4iCount == 0)
      return D3D_OK;

    dirtyFlags |= dirtyFlags::psConstants;
    arrayCopyT(&psConstants.intConstants[StartRegister], pConstantData, Vector4iCount);
    return D3D_OK;
  }

  HRESULT D3D9State::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
    if (pConstantData == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetPixelShaderConstantB: pConstantData was nullptr");

    if (BoolCount == 0)
      return D3D_OK;

    arrayCopyJ(pConstantData, &psConstants.boolConstants[StartRegister], BoolCount);
    return D3D_OK;
  }
  HRESULT D3D9State::SetPixelShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount) {
    if (pConstantData == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "SetPixelShaderConstantB: pConstantData was nullptr");

    if (BoolCount == 0)
      return D3D_OK;

    dirtyFlags |= dirtyFlags::psConstants;
    arrayCopyT(&psConstants.boolConstants[StartRegister], pConstantData, BoolCount);
    return D3D_OK;
  }

  HRESULT D3D9State::GetScissorRect(RECT* pRect) {
    if (pRect == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetScissorRect: pRect was nullptr.");

    *pRect = scissorRect;
    return D3D_OK;
  }
  HRESULT D3D9State::SetScissorRect(CONST RECT* pRect) {
    if (pRect == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "SetScissorRect: pRect was nullptr.");

    scissorRect = *pRect;
    scissorRectCaptured = true;

    dirtyFlags |= dirtyFlags::scissorRect;
    return D3D_OK;
  }

  HRESULT D3D9State::GetViewport(D3DVIEWPORT9* pViewport) {
    if (pViewport == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetViewport: pViewport was nullptr.");

    *pViewport = viewport;
    return D3D_OK;
  }
  HRESULT D3D9State::SetViewport(CONST D3DVIEWPORT9* pViewport) {
    if (pViewport == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "SetViewport: pViewport was nullptr.");

    viewport = *pViewport;
    viewportCaptured = true;

    dirtyFlags |= dirtyFlags::viewport;
    return D3D_OK;
  }

  //

  void D3D9State::captureRenderState(D3DRENDERSTATETYPE state, bool recapture) {
    if (recapture && renderStateCaptures[state] == false)
      return;

    m_device->GetRenderState(state, &renderState[state]);
    renderStateCaptures[state] = true;
  }

  void D3D9State::captureTextureStageState(uint32_t stage, D3DTEXTURESTAGESTATETYPE type, bool recapture) {
    if (recapture && textureStageStateCaptures[stage][type] == false)
      return;

    m_device->GetTextureStageState(stage, type, &textureStageStates[stage][type]);
    textureStageStateCaptures[stage][type] = true;
  }
  void D3D9State::captureSamplerState(uint32_t sampler, D3DSAMPLERSTATETYPE type, bool recapture) {
    if (recapture && samplerStateCaptures[sampler][type] == false)
      return;

    uint32_t internalSampler = 0;
    convert::mapStageToSampler(sampler, &internalSampler);
    m_device->GetSamplerState(sampler, type, &samplerStates[internalSampler][type]);
    samplerStateCaptures[internalSampler][type] = true;
  }

  void D3D9State::capturePixelRenderStates(bool recapture) {
    captureRenderState(D3DRS_ZENABLE, recapture);
    captureRenderState(D3DRS_FILLMODE, recapture);
    captureRenderState(D3DRS_SHADEMODE, recapture);
    captureRenderState(D3DRS_ZWRITEENABLE, recapture);
    captureRenderState(D3DRS_ALPHATESTENABLE, recapture);
    captureRenderState(D3DRS_LASTPIXEL, recapture);
    captureRenderState(D3DRS_SRCBLEND, recapture);
    captureRenderState(D3DRS_DESTBLEND, recapture);
    captureRenderState(D3DRS_ZFUNC, recapture);
    captureRenderState(D3DRS_ALPHAREF, recapture);
    captureRenderState(D3DRS_ALPHAFUNC, recapture);
    captureRenderState(D3DRS_DITHERENABLE, recapture);
    captureRenderState(D3DRS_FOGSTART, recapture);
    captureRenderState(D3DRS_FOGEND, recapture);
    captureRenderState(D3DRS_FOGDENSITY, recapture);
    captureRenderState(D3DRS_ALPHABLENDENABLE, recapture);
    captureRenderState(D3DRS_DEPTHBIAS, recapture);
    captureRenderState(D3DRS_STENCILENABLE, recapture);
    captureRenderState(D3DRS_STENCILFAIL, recapture);
    captureRenderState(D3DRS_STENCILZFAIL, recapture);
    captureRenderState(D3DRS_STENCILPASS, recapture);
    captureRenderState(D3DRS_STENCILFUNC, recapture);
    captureRenderState(D3DRS_STENCILREF, recapture);
    captureRenderState(D3DRS_STENCILMASK, recapture);
    captureRenderState(D3DRS_STENCILWRITEMASK, recapture);
    captureRenderState(D3DRS_TEXTUREFACTOR, recapture);
    captureRenderState(D3DRS_WRAP0, recapture);
    captureRenderState(D3DRS_WRAP1, recapture);
    captureRenderState(D3DRS_WRAP2, recapture);
    captureRenderState(D3DRS_WRAP3, recapture);
    captureRenderState(D3DRS_WRAP4, recapture);
    captureRenderState(D3DRS_WRAP5, recapture);
    captureRenderState(D3DRS_WRAP6, recapture);
    captureRenderState(D3DRS_WRAP7, recapture);
    captureRenderState(D3DRS_WRAP8, recapture);
    captureRenderState(D3DRS_WRAP9, recapture);
    captureRenderState(D3DRS_WRAP10, recapture);
    captureRenderState(D3DRS_WRAP11, recapture);
    captureRenderState(D3DRS_WRAP12, recapture);
    captureRenderState(D3DRS_WRAP13, recapture);
    captureRenderState(D3DRS_WRAP14, recapture);
    captureRenderState(D3DRS_WRAP15, recapture);
    captureRenderState(D3DRS_COLORWRITEENABLE, recapture);
    captureRenderState(D3DRS_BLENDOP, recapture);
    captureRenderState(D3DRS_SCISSORTESTENABLE, recapture);
    captureRenderState(D3DRS_SLOPESCALEDEPTHBIAS, recapture);
    captureRenderState(D3DRS_ANTIALIASEDLINEENABLE, recapture);
    captureRenderState(D3DRS_TWOSIDEDSTENCILMODE, recapture);
    captureRenderState(D3DRS_CCW_STENCILFAIL, recapture);
    captureRenderState(D3DRS_CCW_STENCILZFAIL, recapture);
    captureRenderState(D3DRS_CCW_STENCILPASS, recapture);
    captureRenderState(D3DRS_CCW_STENCILFUNC, recapture);
    captureRenderState(D3DRS_COLORWRITEENABLE1, recapture);
    captureRenderState(D3DRS_COLORWRITEENABLE2, recapture);
    captureRenderState(D3DRS_COLORWRITEENABLE3, recapture);
    captureRenderState(D3DRS_BLENDFACTOR, recapture);
    captureRenderState(D3DRS_SRGBWRITEENABLE, recapture);
    captureRenderState(D3DRS_SEPARATEALPHABLENDENABLE, recapture);
    captureRenderState(D3DRS_SRCBLENDALPHA, recapture);
    captureRenderState(D3DRS_DESTBLENDALPHA, recapture);
    captureRenderState(D3DRS_BLENDOPALPHA, recapture);
  }
  void D3D9State::capturePixelTextureStates(bool recapture) {
    for(uint32_t i = 0; i < 8; i++)
    {
      captureTextureStageState(i, D3DTSS_COLOROP, recapture);
      captureTextureStageState(i, D3DTSS_COLORARG1, recapture);
      captureTextureStageState(i, D3DTSS_COLORARG2, recapture);
      captureTextureStageState(i, D3DTSS_ALPHAOP, recapture);
      captureTextureStageState(i, D3DTSS_ALPHAARG1, recapture);
      captureTextureStageState(i, D3DTSS_ALPHAARG2, recapture);
      captureTextureStageState(i, D3DTSS_BUMPENVMAT00, recapture);
      captureTextureStageState(i, D3DTSS_BUMPENVMAT01, recapture);
      captureTextureStageState(i, D3DTSS_BUMPENVMAT10, recapture);
      captureTextureStageState(i, D3DTSS_BUMPENVMAT11, recapture);
      captureTextureStageState(i, D3DTSS_TEXCOORDINDEX, recapture);
      captureTextureStageState(i, D3DTSS_BUMPENVLSCALE, recapture);
      captureTextureStageState(i, D3DTSS_BUMPENVLOFFSET, recapture);
      captureTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, recapture);
      captureTextureStageState(i, D3DTSS_COLORARG0, recapture);
      captureTextureStageState(i, D3DTSS_ALPHAARG0, recapture);
      captureTextureStageState(i, D3DTSS_RESULTARG, recapture);
    }
  }
  void D3D9State::capturePixelSamplerStates(bool recapture) {
    forEachSampler([&](uint32_t i) {
      captureSamplerState(i, D3DSAMP_ADDRESSU, recapture);
      captureSamplerState(i, D3DSAMP_ADDRESSV, recapture);
      captureSamplerState(i, D3DSAMP_ADDRESSW, recapture);
      captureSamplerState(i, D3DSAMP_BORDERCOLOR, recapture);
      captureSamplerState(i, D3DSAMP_MAGFILTER, recapture);
      captureSamplerState(i, D3DSAMP_MINFILTER, recapture);
      captureSamplerState(i, D3DSAMP_MIPFILTER, recapture);
      captureSamplerState(i, D3DSAMP_MIPMAPLODBIAS, recapture);
      captureSamplerState(i, D3DSAMP_MAXMIPLEVEL, recapture);
      captureSamplerState(i, D3DSAMP_MAXANISOTROPY, recapture);
      captureSamplerState(i, D3DSAMP_SRGBTEXTURE, recapture);
      captureSamplerState(i, D3DSAMP_ELEMENTINDEX, recapture);
    });
  }
  void D3D9State::capturePixelShaderStates(bool recapture) {
    if (recapture && pixelShaderCaptured == false)
      return;

    pixelShaderCaptured = true;

    Com<IDirect3DPixelShader9> tempShader;
    m_device->GetPixelShader(&tempShader);
    SetPixelShader(tempShader.ptr());

    m_device->GetPixelShaderConstantF(0, (float*)&psConstants.floatConstants[0], 256);
    m_device->GetPixelShaderConstantI(0, (int*)&psConstants.intConstants, 16);
    m_device->GetPixelShaderConstantB(0, (int*)&psConstants.boolConstants, 16);
  }
  void D3D9State::captureVertexRenderStates(bool recapture) {
    captureRenderState(D3DRS_CULLMODE, recapture);
    captureRenderState(D3DRS_FOGENABLE, recapture);
    captureRenderState(D3DRS_FOGCOLOR, recapture);
    captureRenderState(D3DRS_FOGTABLEMODE, recapture);
    captureRenderState(D3DRS_FOGSTART, recapture);
    captureRenderState(D3DRS_FOGEND, recapture);
    captureRenderState(D3DRS_FOGDENSITY, recapture);
    captureRenderState(D3DRS_RANGEFOGENABLE, recapture);
    captureRenderState(D3DRS_AMBIENT, recapture);
    captureRenderState(D3DRS_COLORVERTEX, recapture);
    captureRenderState(D3DRS_FOGVERTEXMODE, recapture);
    captureRenderState(D3DRS_CLIPPING, recapture);
    captureRenderState(D3DRS_LIGHTING, recapture);
    captureRenderState(D3DRS_LOCALVIEWER, recapture);
    captureRenderState(D3DRS_EMISSIVEMATERIALSOURCE, recapture);
    captureRenderState(D3DRS_AMBIENTMATERIALSOURCE, recapture);
    captureRenderState(D3DRS_DIFFUSEMATERIALSOURCE, recapture);
    captureRenderState(D3DRS_SPECULARMATERIALSOURCE, recapture);
    captureRenderState(D3DRS_VERTEXBLEND, recapture);
    captureRenderState(D3DRS_CLIPPLANEENABLE, recapture);
    captureRenderState(D3DRS_POINTSIZE, recapture);
    captureRenderState(D3DRS_POINTSIZE_MIN, recapture);
    captureRenderState(D3DRS_POINTSPRITEENABLE, recapture);
    captureRenderState(D3DRS_POINTSCALEENABLE, recapture);
    captureRenderState(D3DRS_POINTSCALE_A, recapture);
    captureRenderState(D3DRS_POINTSCALE_B, recapture);
    captureRenderState(D3DRS_POINTSCALE_C, recapture);
    captureRenderState(D3DRS_MULTISAMPLEANTIALIAS, recapture);
    captureRenderState(D3DRS_MULTISAMPLEMASK, recapture);
    captureRenderState(D3DRS_PATCHEDGESTYLE, recapture);
    captureRenderState(D3DRS_POINTSIZE_MAX, recapture);
    captureRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, recapture);
    captureRenderState(D3DRS_TWEENFACTOR, recapture);
    captureRenderState(D3DRS_POSITIONDEGREE, recapture);
    captureRenderState(D3DRS_NORMALDEGREE, recapture);
    captureRenderState(D3DRS_MINTESSELLATIONLEVEL, recapture);
    captureRenderState(D3DRS_MAXTESSELLATIONLEVEL, recapture);
    captureRenderState(D3DRS_ADAPTIVETESS_X, recapture);
    captureRenderState(D3DRS_ADAPTIVETESS_Y, recapture);
    captureRenderState(D3DRS_ADAPTIVETESS_Z, recapture);
    captureRenderState(D3DRS_ADAPTIVETESS_W, recapture);
    captureRenderState(D3DRS_ENABLEADAPTIVETESSELLATION, recapture);
    captureRenderState(D3DRS_NORMALIZENORMALS, recapture);
    captureRenderState(D3DRS_SPECULARENABLE, recapture);
    captureRenderState(D3DRS_SHADEMODE, recapture);
  }
  void D3D9State::captureVertexSamplerStates(bool recapture) {
    forEachSampler([&](uint32_t i) {
      captureSamplerState(i, D3DSAMP_DMAPOFFSET, recapture);
    });
  }
  void D3D9State::captureVertexTextureStates(bool recapture) {
    for(uint32_t i = 0; i < 8; i++) {
      captureTextureStageState(i, D3DTSS_TEXCOORDINDEX, recapture);
      captureTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, recapture);
    }
  }
  void D3D9State::captureVertexShaderStates(bool recapture) {
    if (recapture && vertexShaderCaptured == false)
      return;

    vertexShaderCaptured = true;
    
    Com<IDirect3DVertexShader9> tempShader;
    m_device->GetVertexShader(&tempShader);
    SetVertexShader(tempShader.ptr());

    m_device->GetVertexShaderConstantF(0, (float*)&vsConstants.floatConstants[0], 256);
    m_device->GetVertexShaderConstantI(0, (int*)&vsConstants.intConstants, 16);
    m_device->GetVertexShaderConstantB(0, (int*)&vsConstants.boolConstants, 16);
  }
  void D3D9State::captureVertexDeclaration(bool recapture) {
    if (recapture && vertexDeclCaptured == false)
      return;

    vertexDeclCaptured = true;

    Com<IDirect3DVertexDeclaration9> tempDecl;
    m_device->GetVertexDeclaration(&tempDecl);
    SetVertexDeclaration(tempDecl.ptr());
  }
  void D3D9State::captureTextures(bool recapture) {
    forEachSampler([&](DWORD i) {
      DWORD sampler = 0;
      convert::mapStageToSampler(i, &sampler);

      if (recapture && textureCaptured[sampler] == false)
        return;

      textureCaptured[sampler] = true;

      Com<IDirect3DBaseTexture9> tempTexture;
      m_device->GetTexture(i, &tempTexture);
      SetTexture(i, tempTexture.ptr());
    });
  }

  void D3D9State::captureVertexStreams(bool recapture) {
    for(uint32_t i = 0; i < vertexBuffers.size(); i++) {
      if (recapture && vertexBufferCaptures[i] == false)
        continue;
      vertexBufferCaptures[i] = true;

      Com<IDirect3DVertexBuffer9> tempBuffer;
      UINT offset;
      UINT stride;
      m_device->GetStreamSource(i, &tempBuffer, &offset, &stride);
      SetStreamSource(i, tempBuffer.ptr(), offset, stride);
    }
  }

  void D3D9State::captureIndexBuffer(bool recapture) {
    if (recapture && indexBufferCaptured == false)
      return;

    indexBufferCaptured = true;

    Com<IDirect3DIndexBuffer9> tempBuffer;
    m_device->GetIndices(&tempBuffer);
    SetIndices(tempBuffer.ptr());
  }

  void D3D9State::captureViewport(bool recapture) {
    if (recapture && viewportCaptured == false)
      return;

    viewportCaptured = true;
    m_device->GetViewport(&viewport);
  }

  void D3D9State::captureScissor(bool recapture) {
    if (recapture && scissorRectCaptured == false)
      return;

    scissorRectCaptured = true;
    m_device->GetScissorRect(&scissorRect);
  }

  // StateBlock

  Direct3DStateBlock9::Direct3DStateBlock9(Direct3DDevice9Ex* device, uint32_t stateBlockState)
    : D3D9DeviceUnknown<IDirect3DStateBlock9>(device)
    , m_state{ device, stateBlockState } {}

  HRESULT STDMETHODCALLTYPE Direct3DStateBlock9::QueryInterface(REFIID riid, LPVOID* ppv) {
    InitReturnPtr(ppv);

    if (!ppv)
      return E_POINTER;

    if (riid == __uuidof(IDirect3DStateBlock9) || riid == __uuidof(IUnknown)) {
      *ppv = ref(this);
      return D3D_OK;
    }

    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE Direct3DStateBlock9::Capture() {
    CriticalSection cs(m_device);
    m_state.capture(D3DSBT_ALL, true);
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DStateBlock9::Apply() {
    CriticalSection cs(m_device);
    m_state.apply();
    return D3D_OK;
  }
}