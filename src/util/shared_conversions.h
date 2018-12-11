#include <d3d9.h>
#include <string>

namespace dxapex {

    namespace convert {

        const std::string& declUsage(bool vsInput, D3DDECLUSAGE usage);
        uint32_t sysValue(bool vsInput, D3DDECLUSAGE usage);

    }

}