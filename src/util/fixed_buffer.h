#pragma once

namespace dxapex {

  template <size_t Capacity, typename T>
  class FixedBuffer {

  public:

    void push(T token) {
      m_tokens[m_size++] = token;
    }

    size_t size() {
      return m_size;
    }

    T& get(size_t index) {
      return m_tokens[index];
    }

  private:

    T m_tokens[Capacity] = { 0 };
    size_t m_size = 0;
  };

}