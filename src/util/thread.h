#pragma once

#include <Windows.h>

namespace dxup {

  class Mutex {
  public:
    Mutex() {
      m_mutex = CreateMutex(nullptr, false, nullptr);
    }

    ~Mutex() {
      unlock();
      CloseHandle(m_mutex);
    }

    inline void lock() {
      WaitForSingleObject(m_mutex, INFINITE);
    }

    inline void unlock() {
      ReleaseMutex(m_mutex);
    }

  private:
    HANDLE m_mutex;
  };

  class Lock {
  public:
    Lock(Mutex& mutex)
      : m_mutex{ mutex } {
      m_mutex.lock();
    }

    ~Lock() {
      m_mutex.unlock();
    }
  private:
    Mutex& m_mutex;
  };

}