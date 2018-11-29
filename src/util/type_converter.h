#pragma once

namespace dxapex {

  template <typename T, typename J, size_t Size>
  class TypeConverter {

  public:

    struct TypeMapping {
      T Type1;
      J Type2;
    };

    TypeConverter(TypeMapping map[Size]) {
      std::memcpy(m_map, map, sizeof(TypeMapping) * Size);
    }

    T toT(J j) {
      for (size_t i = 0; i < Size; i++)
      {
        if (m_map[i].Type2 == j)
          return m_map[i].Type1;
      }
      return m_map[0].Type1;
    }

    J toJ(T t) {
      for (size_t i = 0; i < Size; i++)
      {
        if (m_map[i].Type1 == t)
          return m_map[i].Type2;
      }
      return m_map[0].Type2;
    }

  private:

    TypeMapping m_map[Size];

  };

}