#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include "windows_includes.h"
#include "../../version.h"

#include <shlwapi.h>

namespace dxup {

  namespace log {

    FILE* logFile = nullptr;
    bool fileValid = false;

    void createLog() {
      char logName[MAX_PATH];
      char exePath[MAX_PATH];

      GetModuleFileNameA(NULL, exePath, MAX_PATH);
      PathRemoveExtensionA(exePath);
      const char* exeName = PathFindFileNameA(exePath);

      snprintf(logName, MAX_PATH, "%s_d3d9.log", exeName);

      logFile = fopen(logName, "w");

      fileValid = logFile != nullptr;

      if (fileValid)
        fprintf(logFile, u8"DXUP - 🐸 - Version %s\n", DXUP_VERSION);
    }

    void internal_write(const char* prefix, const char* fmt, va_list list) {
      if (logFile == nullptr)
        createLog();

      char buffer[1024];
      char buffer2[1024];
      vsnprintf(buffer, 1024, fmt, list);

      snprintf(buffer2, 1024, "[%s] %s\n", prefix, buffer);

      if (fileValid) {
        fprintf(logFile, "%s", buffer2);
        fflush(logFile);
      }
      
      OutputDebugStringA(buffer2);
    }

  }

}