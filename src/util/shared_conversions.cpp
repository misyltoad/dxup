#include "shared_conversions.h"
#include <array>

namespace dxapex {

  namespace convert {

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

    const std::string& declUsage(D3DDECLUSAGE usage) {
      if (usage > 14 || usage < 0)
        return dclUsageMap[14];

      return dclUsageMap[usage];
    }

  }

}