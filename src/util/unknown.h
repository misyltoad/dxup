#pragma once

#include "d3d9_includes.h"
#include "d3d9_definitions.h"

#include <atomic>

namespace dxup {

  // Double ref-count tip from DXVK.
  // https://github.com/doitsujin/dxvk/blob/master/src/util/com/com_object.h
  template <typename... Base>
  class Unknown : public Base... {

  public:

    virtual ~Unknown() {}

    ULONG STDMETHODCALLTYPE AddRef() {
      ULONG refCount = m_refCount++;

      return refCount;
    }

    ULONG STDMETHODCALLTYPE Release() {
      ULONG refCount = --m_refCount;
      if (refCount == 0 && m_refPrivate == 0)
        delete this;
      
      return refCount;
    }

    void AddRefPrivate() {
      m_refPrivate++;
    }

    void ReleasePrivate() {
      m_refPrivate--;

      if (m_refCount == 0 && m_refPrivate == 0)
        delete this;
    }

  private:
    std::atomic<ULONG> m_refCount = { 0ul };
    std::atomic<ULONG> m_refPrivate = { 0ul };
  };

  // COM Automatic Reference Counter from DXVK.
  // https://github.com/doitsujin/dxvk/blob/master/src/util/com/com_pointer.h
  template<typename T, bool Private>
  class ComBase {

  public:
    
    ComBase() { }
    ComBase(std::nullptr_t) { }
    ComBase(T* object)
    : m_ptr(object) {
      this->incRef();
    }
    
    ComBase(const ComBase& other)
    : m_ptr(other.m_ptr) {
      this->incRef();
    }
    
    ComBase(ComBase&& other)
    : m_ptr(other.m_ptr) {
      other.m_ptr = nullptr;
    }
    
    ComBase& operator = (T* object) {
      this->decRef();
      m_ptr = object;
      this->incRef();
      return *this;
    }
    
    ComBase& operator = (const ComBase& other) {
      other.incRef();
      this->decRef();
      m_ptr = other.m_ptr;
      return *this;
    }
    
    ComBase& operator = (ComBase&& other) {
      this->decRef();
      this->m_ptr = other.m_ptr;
      other.m_ptr = nullptr;
      return *this;
    }
    
    ComBase& operator = (std::nullptr_t) {
      this->decRef();
      m_ptr = nullptr;
      return *this;
    }
    
    ~ComBase() {
      this->decRef();
    }
    
    T* operator -> () const {
      return m_ptr;
    }
    
    T**       operator & ()       { return &m_ptr; }
    T* const* operator & () const { return &m_ptr; }
    
    bool operator == (const ComBase<T, Private>& other) const { return m_ptr == other.m_ptr; }
    bool operator != (const ComBase<T, Private>& other) const { return m_ptr != other.m_ptr; }
    
    bool operator == (const T* other) const { return m_ptr == other; }
    bool operator != (const T* other) const { return m_ptr != other; }
    
    bool operator == (std::nullptr_t) const { return m_ptr == nullptr; }
    bool operator != (std::nullptr_t) const { return m_ptr != nullptr; }
    
    T* ref() const {
      this->incRef();
      return m_ptr;
    }
    
    T* ptr() const {
      return m_ptr;
    }
    
  private:
    
    T* m_ptr = nullptr;
    
    void incRef() const {
      if (m_ptr != nullptr) {
        if constexpr (Private)
          m_ptr->AddRefPrivate();
        else
          m_ptr->AddRef();
      }
    }
    
    void decRef() const {
      if (m_ptr != nullptr) {
        if constexpr (Private)
          m_ptr->ReleasePrivate();
        else
          m_ptr->Release();
      }
    }
    
  };

  template <typename T>
  using Com = ComBase<T, false>;

  template <typename T>
  using ComPrivate = ComBase<T, true>;
  
  template<typename T>
  T* ref(T* object) {
    if (object != nullptr)
      object->AddRef();
    return object;
  }

  template<typename T>
  T* ref(Com<T> object) {
	if (object != nullptr)
		object->AddRef();
	  return object.ptr();
  }

  template<typename T>
  T* ref(ComPrivate<T> object) {
    if (object != nullptr)
      object->AddRef();
    return object.ptr();
  }
}