#pragma once

#include "d3d9_base.h"

namespace dxapex {

  UINT alignPitch(UINT pitch);

  UINT calculatePitch(DXGI_FORMAT format, UINT width);

}