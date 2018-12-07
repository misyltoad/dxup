#include "d3dcompiler_helpers.h"

namespace dxapex {

  namespace d3dcompiler {

    HMODULE d3dcompilerModule = nullptr;

    void loadModule() {
      if (d3dcompilerModule == nullptr)
        d3dcompilerModule = LoadLibraryA("d3dcompiler.dll");

      if (d3dcompilerModule == nullptr)
          d3dcompilerModule = LoadLibraryA("d3dcompiler_47.dll");

      if (d3dcompilerModule == nullptr)
          d3dcompilerModule = LoadLibraryA("d3dcompiler_43.dll");
    }

    bool disassemble(HRESULT* result, LPCVOID pSrcData, SIZE_T SrcDataSize, UINT Flags, LPCSTR szComments, ID3DBlob** ppDisassembly) {
      loadModule();

      if (!d3dcompilerModule)
        return false;

      typedef HRESULT(WINAPI *D3DDisassembleFunc)(LPCVOID pSrcData, SIZE_T SrcDataSize, UINT Flags, LPCSTR szComments, ID3DBlob** ppDisassembly);
      D3DDisassembleFunc D3DDisassembleDynamic = (D3DDisassembleFunc)GetProcAddress(d3dcompilerModule, "D3DDisassemble");

      HRESULT foundResult = D3DDisassembleDynamic(pSrcData, SrcDataSize, Flags, szComments, ppDisassembly);
      *result = foundResult;

      return true;
    }

  }

}