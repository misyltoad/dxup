#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include "windows_includes.h"

namespace dxapex {

  namespace log {

    FILE* logFile = nullptr;

    void internal_write(const char* prefix, const char* fmt, va_list list) {
      if (logFile == nullptr) {
        logFile = fopen("log_d3d9.txt", "w");
      }

      char buffer[1024];
      char buffer2[1024];
      vsnprintf(buffer, 1024, fmt, list);

      snprintf(buffer2, 1024, "[%s] %s\n", prefix, buffer);

      fprintf(logFile, "%s", buffer2);
      fflush(logFile);
      
      OutputDebugStringA(buffer2);
    }

  }

}