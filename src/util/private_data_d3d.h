#pragma once

#include "windows_includes.h"
#include <list>
#include <memory>

namespace dxup {

  class PrivateDataD3D {

  public:

    HRESULT SetPrivateData(REFGUID refguid, const void *pData, DWORD SizeOfData, DWORD Flags);
    HRESULT GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData);
    HRESULT FreePrivateData(REFGUID refguid);

  private:

    struct Entry {
      GUID guid;
      void* data;
      DWORD size;
      UINT flags;
    };

    HRESULT FreeEntry(Entry& entry);

    std::list<Entry> m_entries;
    
  };

}