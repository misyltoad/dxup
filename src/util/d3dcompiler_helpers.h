#pragma once

#include <d3dcompiler.h>

namespace dxapex {

  namespace d3dcompiler {

    bool disassemble(HRESULT* result, LPCVOID pSrcData, SIZE_T SrcDataSize, UINT Flags, LPCSTR szComments, ID3DBlob** ppDisassembly);

  }

}