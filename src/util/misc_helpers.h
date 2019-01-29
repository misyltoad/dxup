#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstring>

namespace dxup {

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
    return lastPtr(vec) + 1;
  }

  template <typename T>
  constexpr T alignTo(T num, T align) {
    return ((num + align - 1) / align) * align;
  }

  template <typename T>
  constexpr T alignDown(T num, T align) {
    return (num / align) * align;
  }

  template <typename T>
  void pushAlignedString(std::vector<T>& vec, const char* str, size_t len) {
    len++; // for NULL terminator
    size_t paddedLen = alignTo(len, sizeof(T));

    uint32_t* end = nextPtr(vec);
    vec.resize(vec.size() + (paddedLen / sizeof(T)));
    std::memset(end, 0xAB, paddedLen); // FXC pads with 0xAB
    std::memcpy(end, str, len);
  }

  template <typename T>
  void pushAlignedString(std::vector<T>& vec, const std::string& str) {
    pushAlignedString(vec, str.c_str(), str.length());
  }

  template <typename T, typename J>
  void arrayCopyT(T* dst, const J* src, size_t tCount) {
    std::memcpy((void*)dst, (const void*)src, sizeof(T) * tCount);
  }

  template <typename T, typename J>
  void arrayCopyJ(T* dst, const J* src, size_t jCount) {
    std::memcpy((void*)dst, (const void*)src, sizeof(J) * jCount);
  }

}