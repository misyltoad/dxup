#pragma once

#include <vector>

namespace dxup {

  template <typename T, typename J>
  class TypeConverter {

  public:

    struct TypeMapping {
      T Type1;
      J Type2;
    };

    TypeConverter(const std::vector<TypeMapping>& map)
      : m_map(map) {}

    T toT(J j) {
      for (size_t i = 0; i < m_map.size(); i++)
      {
        if (m_map[i].Type2 == j)
          return m_map[i].Type1;
      }
      return m_map[0].Type1;
    }

    J toJ(T t) {
      for (size_t i = 0; i < m_map.size(); i++)
      {
        if (m_map[i].Type1 == t)
          return m_map[i].Type2;
      }
      return m_map[0].Type2;
    }

  private:

    const std::vector<TypeMapping>& m_map;

  };

}