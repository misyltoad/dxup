#pragma once

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

}