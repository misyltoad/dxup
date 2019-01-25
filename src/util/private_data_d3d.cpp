#include "private_data_d3d.h"
#include <d3d9.h>
#include <cstring>

namespace dxup {

  HRESULT PrivateDataD3D::SetPrivateData(REFGUID refguid, const void *pData, DWORD SizeOfData, DWORD Flags) {
    if (pData == nullptr || SizeOfData == 0)
      return D3DERR_INVALIDCALL;

    if (Flags & D3DSPD_IUNKNOWN) {
      if (SizeOfData != sizeof(IUnknown*))
        return D3DERR_INVALIDCALL;

      IUnknown* unknown = (IUnknown*)pData;
      unknown->AddRef();
    }

    Entry newEntry;
    newEntry.guid = refguid;
    newEntry.flags = Flags;
    newEntry.size = SizeOfData;
    newEntry.data = (void*) new uint8_t[SizeOfData];
    std::memcpy(newEntry.data, pData, SizeOfData);

    std::list<Entry>::iterator iter;
    for (iter = m_entries.begin(); iter != m_entries.end(); iter++) {
      Entry& entry = *iter;
      
      if (IsEqualGUID(entry.guid, refguid)) {
        FreeEntry(entry);

        entry = newEntry;
        return D3D_OK;
      }
    }

    m_entries.push_back(newEntry);

    return D3D_OK;
  }

  HRESULT PrivateDataD3D::GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData) {
    if (pSizeOfData == nullptr)
      return D3DERR_INVALIDCALL;

    std::list<Entry>::iterator iter;
    for (iter = m_entries.begin(); iter != m_entries.end(); iter++) {
      Entry& entry = *iter;
      
      if (IsEqualGUID(entry.guid, refguid)) {
        if (entry.flags & D3DSPD_IUNKNOWN) {
          IUnknown* unknown = (IUnknown*)pData;
          unknown->AddRef();
        }

        *pSizeOfData = entry.size;

        if (pData != nullptr && *pSizeOfData >= entry.size) {
          std::memcpy(pData, entry.data, entry.size);
          return D3D_OK;
        }
        else
          return D3DERR_MOREDATA;
      }
    }

    return D3DERR_NOTFOUND;
  }

  HRESULT PrivateDataD3D::FreePrivateData(REFGUID refguid) {
    std::list<Entry>::iterator iter;
    for (iter = m_entries.begin(); iter != m_entries.end(); iter++) {
      Entry& entry = *iter;

      if (IsEqualGUID(entry.guid, refguid))
        return FreeEntry(entry);
    }

    return D3DERR_NOTFOUND;
  }

  HRESULT PrivateDataD3D::FreeEntry(Entry& entry) {

    if (entry.data != nullptr) {

      if (entry.flags & D3DSPD_IUNKNOWN) {
        IUnknown* unknown = reinterpret_cast<IUnknown*>(entry.data);
        unknown->Release();
      }
      
      uint8_t* data = reinterpret_cast<uint8_t*>(entry.data);
      delete data;
    }

    return D3D_OK;
  }
}