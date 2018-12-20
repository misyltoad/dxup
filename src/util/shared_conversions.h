#include <d3d9.h>
#include <string>

namespace dxup {

    namespace convert {

        const std::string& declUsage(bool vsInput, bool target, D3DDECLUSAGE usage);
        uint32_t sysValue(bool vsInput, bool target, D3DDECLUSAGE usage);

    }

}