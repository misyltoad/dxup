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
  T* lastPtr(std::vector<T>& vec) {
    return &vec[vec.size() - 1];
  }

  template <typename T>
  T* nextPtr(std::vector<T>& vec) {
    return &vec[vec.size()];
  }

  template <typename T>
  void pushAlignedString(std::vector<T>& vec, const std::string& str) {
    size_t len = str.length() + 1; // for NULL terminator
    size_t paddedLen = len + len % sizeof(T);

    uint32_t* end = nextPtr(vec);
    vec.resize( vec.size() + (paddedLen / sizeof(T)) );
    std::memset(end, 0xAB, paddedLen); // FXC pads with 0xAB
    std::memcpy(end, str.c_str(), len);
  }

}