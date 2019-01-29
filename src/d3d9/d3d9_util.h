#pragma once

#include "d3d9_includes.h"
#include "../util/log.h"
#include "../util/shared_conversions.h"

namespace dxup {

  namespace convert {
    DXGI_FORMAT format(D3DFORMAT Format);
    D3DFORMAT format(DXGI_FORMAT Format);

    DXGI_FORMAT makeUntypeless(DXGI_FORMAT format, bool srgb);
    DXGI_FORMAT makeTypeless(DXGI_FORMAT format);

    DXGI_FORMAT makeSwapchainCompliant(DXGI_FORMAT format);

    DXGI_MODE_SCANLINE_ORDER scanlineOrdering(D3DSCANLINEORDERING ScanlineOrdering);
    D3DSCANLINEORDERING scanlineOrdering(DXGI_MODE_SCANLINE_ORDER ScanlineOrdering);

    UINT cpuFlags(D3DPOOL pool, UINT usage, D3DRESOURCETYPE type);
    D3D11_USAGE usage(D3DPOOL pool, UINT usage, D3DRESOURCETYPE type);

    UINT primitiveData(D3DPRIMITIVETYPE type, UINT count, D3D_PRIMITIVE_TOPOLOGY& topology);

    DXGI_FORMAT declType(D3DDECLTYPE type);

    void color(D3DCOLOR color, FLOAT* d3d11Color);

    inline D3D11_CULL_MODE cullMode(DWORD mode) {
      switch (mode) {
      case D3DCULL_NONE: return D3D11_CULL_NONE;
      case D3DCULL_CW: return D3D11_CULL_FRONT;
      default:
      case D3DCULL_CCW: return D3D11_CULL_BACK;
      }
    }

    inline D3D11_FILL_MODE fillMode(DWORD mode) {
      switch (mode) {
      case D3DFILL_POINT: return D3D11_FILL_WIREFRAME;
      case D3DFILL_WIREFRAME: return D3D11_FILL_WIREFRAME;
      default:
      case D3DFILL_SOLID: return D3D11_FILL_SOLID;
      }
    }

    inline D3D11_STENCIL_OP stencilOp(DWORD op) {
      switch (op) {
      default:
      case D3DSTENCILOP_KEEP: return D3D11_STENCIL_OP_KEEP;
      case D3DSTENCILOP_ZERO: return D3D11_STENCIL_OP_ZERO;
      case D3DSTENCILOP_REPLACE: return D3D11_STENCIL_OP_REPLACE;
      case D3DSTENCILOP_INCRSAT: return D3D11_STENCIL_OP_INCR_SAT;
      case D3DSTENCILOP_DECRSAT: return D3D11_STENCIL_OP_DECR_SAT;
      case D3DSTENCILOP_INVERT: return D3D11_STENCIL_OP_INVERT;
      case D3DSTENCILOP_INCR: return D3D11_STENCIL_OP_INCR;
      case D3DSTENCILOP_DECR: return D3D11_STENCIL_OP_DECR;
      }
    }

    inline D3D11_COMPARISON_FUNC func(DWORD op) {
      switch (op) {
      case D3DCMP_NEVER: return D3D11_COMPARISON_NEVER;
      case D3DCMP_LESS: return D3D11_COMPARISON_LESS;
      case D3DCMP_EQUAL: return D3D11_COMPARISON_EQUAL;
      case D3DCMP_LESSEQUAL: return D3D11_COMPARISON_LESS_EQUAL;
      case D3DCMP_GREATER: return D3D11_COMPARISON_GREATER;
      case D3DCMP_NOTEQUAL: return D3D11_COMPARISON_NOT_EQUAL;
      case D3DCMP_GREATEREQUAL: return D3D11_COMPARISON_GREATER_EQUAL;
      default:
      case D3DCMP_ALWAYS: return D3D11_COMPARISON_ALWAYS;
      }
    }

    inline D3D11_TEXTURE_ADDRESS_MODE textureAddressMode(DWORD mode) {
      switch (mode) {
      default:
      case D3DTADDRESS_WRAP: return D3D11_TEXTURE_ADDRESS_WRAP;
      case D3DTADDRESS_MIRROR: return D3D11_TEXTURE_ADDRESS_MIRROR;
      case D3DTADDRESS_CLAMP: return D3D11_TEXTURE_ADDRESS_CLAMP;
      case D3DTADDRESS_BORDER: return D3D11_TEXTURE_ADDRESS_BORDER;
      case D3DTADDRESS_MIRRORONCE: return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
      }
    }

    inline D3D11_BLEND_OP blendOp(DWORD op) {
      switch (op) {
      default:
      case D3DBLENDOP_ADD: return D3D11_BLEND_OP_ADD;
      case D3DBLENDOP_SUBTRACT: return D3D11_BLEND_OP_SUBTRACT;
      case D3DBLENDOP_REVSUBTRACT: return D3D11_BLEND_OP_REV_SUBTRACT;
      case D3DBLENDOP_MIN: return D3D11_BLEND_OP_MIN;
      case D3DBLENDOP_MAX: return D3D11_BLEND_OP_MAX;
      }
    }

    inline D3D11_BLEND blend(DWORD blend) {
      switch (blend) {
      case D3DBLEND_ZERO: return D3D11_BLEND_ZERO;
      default:
      case D3DBLEND_ONE: return D3D11_BLEND_ONE;
      case D3DBLEND_SRCCOLOR: return D3D11_BLEND_SRC_COLOR;
      case D3DBLEND_INVSRCCOLOR: return D3D11_BLEND_INV_SRC_COLOR;
      case D3DBLEND_SRCALPHA: return D3D11_BLEND_SRC_ALPHA;
      case D3DBLEND_INVSRCALPHA: return D3D11_BLEND_INV_SRC_ALPHA;
      case D3DBLEND_DESTALPHA: return D3D11_BLEND_DEST_ALPHA;
      case D3DBLEND_INVDESTALPHA: return D3D11_BLEND_INV_DEST_ALPHA;
      case D3DBLEND_DESTCOLOR: return D3D11_BLEND_DEST_COLOR;
      case D3DBLEND_INVDESTCOLOR: return D3D11_BLEND_INV_DEST_COLOR;
      case D3DBLEND_SRCALPHASAT: return D3D11_BLEND_SRC_ALPHA_SAT;
        // TODO(Josh): Look into the both variants.
      case D3DBLEND_BOTHSRCALPHA: return D3D11_BLEND_SRC_ALPHA;
      case D3DBLEND_BOTHINVSRCALPHA: return D3D11_BLEND_INV_SRC_ALPHA;

      case D3DBLEND_BLENDFACTOR: return D3D11_BLEND_BLEND_FACTOR;
      case D3DBLEND_INVBLENDFACTOR: return D3D11_BLEND_INV_BLEND_FACTOR;

      case D3DBLEND_SRCCOLOR2: return D3D11_BLEND_SRC1_COLOR;
      case D3DBLEND_INVSRCCOLOR2: return D3D11_BLEND_INV_SRC1_COLOR;
      }
    }

    inline D3D11_FILTER_TYPE filterType(DWORD filter, bool& anisotropic) {
      switch (filter) {
      case D3DTEXF_NONE:
      case D3DTEXF_POINT: return D3D11_FILTER_TYPE_POINT;
      default:
      case D3DTEXF_LINEAR: return D3D11_FILTER_TYPE_LINEAR;
      case D3DTEXF_ANISOTROPIC: anisotropic = true; return D3D11_FILTER_TYPE_LINEAR;
      }
    }

    inline D3D11_FILTER filter(DWORD magFilter, DWORD minFilter, DWORD mipFilter) {
      bool anisotropic = false;
      D3D11_FILTER_TYPE magType = filterType(magFilter, anisotropic);
      D3D11_FILTER_TYPE minType = filterType(minFilter, anisotropic);
      D3D11_FILTER_TYPE mipType = filterType(mipFilter, anisotropic);

      if (anisotropic)
        return D3D11_ENCODE_ANISOTROPIC_FILTER(D3D11_FILTER_REDUCTION_TYPE_STANDARD);
      else
        return D3D11_ENCODE_BASIC_FILTER(minType, magType, mipType, D3D11_FILTER_REDUCTION_TYPE_STANDARD);
    }

    template <typename T>
    HRESULT mapStageToSampler(T Stage, T* Sampler) {
      if ((Stage >= 16 && Stage <= D3DDMAPSAMPLER) || Stage > D3DVERTEXTEXTURESAMPLER3)
        return log::d3derr(D3DERR_INVALIDCALL, "mapStageToSampler: sampler index out of bounds: %d", Stage);

      // For vertex samplers.
      if (Stage >= D3DVERTEXTEXTURESAMPLER0)
        Stage = 16 + (Stage - D3DVERTEXTEXTURESAMPLER0);

      *Sampler = Stage;

      return D3D_OK;
    }

    inline void wideStringToMultiByte(const WCHAR* wide, char* mb, uint32_t mblen) {
      WideCharToMultiByte(CP_ACP, 0, wide, -1, mb, mblen, nullptr, nullptr);
    }
  }

  namespace reinterpret {
    inline float dwordToFloat(DWORD val) {
      return (float&)val;
    }

    inline DWORD floatToDword(float val) {
      return (DWORD&)val;
    }
  }

  template <typename T>
  void makeStagingDesc(T& desc, UINT d3d9Usage, D3DFORMAT format) {
    desc.CPUAccessFlags = d3d9Usage & D3DUSAGE_WRITEONLY ? D3D11_CPU_ACCESS_WRITE : D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    if (format == D3DFMT_R8G8B8)
      desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;
  }

  inline bool isRectDegenerate(const RECT& rect) {
    return rect.top == 0 && rect.right == 0 && rect.left == 0 && rect.bottom == 0;
  }

  inline bool isBoxDegenerate(const D3DBOX& box) {
    return box.Top == 0 && box.Right == 0 && box.Left == 0 && box.Bottom == 0 && box.Front == 0 && box.Back == 0;
  }

  template <typename T, typename J>
  T* useAs(J* obj) {
    return reinterpret_cast<T*>(obj);
  }

  template <typename T>
  inline void forEachSampler(T func) {
    for (uint32_t i = 0; i <= D3DVERTEXTEXTURESAMPLER3; i = (i != 15) ? (i + 1) : D3DVERTEXTEXTURESAMPLER0)
      func(i);
  }

}