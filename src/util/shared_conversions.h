#include <d3d9.h>
#include <string>

namespace dxapex {

    namespace convert {

        const std::string& declUsage(D3DDECLUSAGE usage);
        uint32_t sysValue(D3DDECLUSAGE usage);

    }

}