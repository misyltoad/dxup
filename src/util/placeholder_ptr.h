#pragma once

#include "log.h"

namespace dxup {

  template <typename T>
  class PlaceholderPtr {

  public:

    template <typename J>
    PlaceholderPtr(const char* name, J* ptr) 
      : m_name{ name }
      , m_ptr{ (T*) ptr }
      , m_set{ false }
    {}

    inline const T& getValue() const {
      return *m_ptr;
    }

    inline T& setValue(const T& val) {
      *m_ptr = val;
      m_set = true;
      return *m_ptr;
    }

    inline const T& operator*() const {
      return getValue();
    }

    inline T& operator=(const T& val) {
      return setValue(val);
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