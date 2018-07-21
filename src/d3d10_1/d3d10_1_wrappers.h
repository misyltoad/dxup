#pragma once

#include "d3d10_1_include.h"

namespace dxup
{
    namespace Wrap
    {
        ID3D10Texture2D* Texture2D(ID3D11Texture2D* pTexture);
    }
}