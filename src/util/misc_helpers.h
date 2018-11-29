#pragma once

namespace dxapex {

  template <typename T>
  void InitReturnPtr(T* ptr) {
    if (ptr)
      *ptr = nullptr;
  }

}