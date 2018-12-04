#pragma once

#include "log.h"

namespace dxapex {

  template <typename T>
  class PlaceholderPtr {

  public:

    template <typename J>
    PlaceholderPtr(const char* name, J* ptr) 
      : m_name{ name }
      , m_ptr{ (T*) ptr }
      , m_set{ false }
    {}

    const T& getValue() const {
      return *m_ptr;
    }

    void setValue(const T& val) {
      *m_ptr = val;
      m_set = true;
    }

    ~PlaceholderPtr() {
      if (!m_set)
        log::fail("PlaceholderPtr for %s not set before destruction!", m_name);
    }

  private:
    const char* m_name;
    bool m_set;
    T* m_ptr;
  };

}