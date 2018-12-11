#include "shared_conversions.h"
#include <array>
#include <d3dcommon.h>

namespace dxapex {

  namespace convert {

    std::array<std::string, 15> dclUsageMapSemantic = {
      "SV_Position",
      "BLENDWEIGHT",
      "BLENDINDICES",
      "NORMAL",
      "PSIZE",
      "TEXCOORD",
      "TANGENT",
      "BINORMAL",
      "SV_TessFactor",
      "POSITIONT",
      "COLOR",
      "FOG",
      "SV_Depth",
      "SV_SampleIndex",
      "UNKNOWN"
    };

    std::array<std::string, 15> dclUsageMap = {
      "POSITION",
      "BLENDWEIGHT",
      "BLENDINDICES",
      "NORMAL",
      "PSIZE",
      "TEXCOORD",
      "TANGENT",
      "BINORMAL",
      "TESSFACTOR",
      "POSITIONT",
      "COLOR",
      "FOG",
      "DEPTH",
      "SAMPLE",
      "UNKNOWN"
    };


    const std::string& declUsage(bool vsInput, D3DDECLUSAGE usage) {
      if (usage > 14 || usage < 0)
        return dclUsageMap[14];

      return vsInput ? dclUsageMap[usage] : dclUsageMapSemantic[usage];
    }

    uint32_t sysValue(bool vsInput, D3DDECLUSAGE usage) {
      if (vsInput)
        return D3D_NAME_UNDEFINED;

      switch (usage) {
      case D3DDECLUSAGE_POSITION: return D3D_NAME_POSITION;
      case D3DDECLUSAGE_TESSFACTOR: return D3D_NAME_FINAL_TRI_EDGE_TESSFACTOR;
      case D3DDECLUSAGE_DEPTH: return D3D_NAME_DEPTH;
      case D3DDECLUSAGE_SAMPLE: return D3D_NAME_SAMPLE_INDEX;
      default: return D3D_NAME_UNDEFINED;
      }
    }

  }

}