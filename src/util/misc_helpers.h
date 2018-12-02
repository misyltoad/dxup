#pragma once

#include <string>
#include <vector>
#include <memory>

namespace dxapex {

  template <typename T>
  void InitReturnPtr(T* ptr) {
    if (ptr)
      *ptr = nullptr;
  }

  template <typename T, typename J>
  void pushObject(std::vector<J>& vec, T& thing) {
    J* ptrThing = (J*)&thing;
    for (J i = 0; i < sizeof(T) / sizeof(J); i++)
      vec.push_back(*(ptrThing + i));
  }

  template <typename T>
  void pushAlignedString(std::vector<T>& vec, std::string str) {
    size_t len = str.length();
    len += len % sizeof(T);

    size_t curSize = data.size();
    data.resize(vec.size() + (len / sizeof(T)) );
    std::memset(&data[curSize], 0, len);
    std::memcpy(&data[curSize], str.base(), str.length());
  }

}