#include "thread.h"

namespace dxup {

  static bool mutexesEnabled = false;

  void EnableMutexes() {
    mutexesEnabled = true;
  }

  bool MutexesEnabled() {
    return mutexesEnabled;
  }

}