#pragma once

namespace dxup {

  template <size_t Capacity, typename T>
  class FixedBuffer {

  public:

    void push_back(const T& token) {
      m_tokens[m_size++] = token;
    }

    size_t size() {
      return m_size;
    }

    T& get(size_t index) {
      return m_tokens[index];
    }

  private:

    T m_tokens[Capacity] = { };
    size_t m_size = 0;
  };

}