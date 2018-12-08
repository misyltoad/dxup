#include "shared_conversions.h"
#include <array>

namespace dxapex {

  namespace convert {

    std::array<std::string, 15> dclUsageMap = {
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

    const std::string& declUsage(D3DDECLUSAGE usage) {
      if (usage > 14 || usage < 0)
        return dclUsageMap[14];

      return dclUsageMap[usage];
    }

  }

}