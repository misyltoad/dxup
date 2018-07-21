#include "d3d10_1_wrappers.h"
#include "d3d10_1_texture.h"

namespace dxup
{
    namespace Wrap
    {
        ID3D10Texture2D* Texture2D(ID3D11Texture2D* pTexture)
        {
            return new D3D10Texture2D(pTexture);
        }
    }
}