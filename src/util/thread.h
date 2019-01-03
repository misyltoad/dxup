#pragma once

#include <Windows.h>

namespace dxup {

  bool MutexesEnabled();
  void EnableMutexes();

  class Mutex {
  public:
    Mutex() {
      if (MutexesEnabled())
        m_mutex = CreateMutex(nullptr, false, nullptr);
    }

    ~Mutex() {
      if (MutexesEnabled()) {
        unlock();
        CloseHandle(m_mutex);
      }
    }

    inline void lock() {
      if (MutexesEnabled())
        WaitForSingleObject(m_mutex, INFINITE);
    }

    inline void unlock() {
      if (MutexesEnabled())
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