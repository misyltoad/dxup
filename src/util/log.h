#pragma once
#include <stdarg.h>

#include "config.h"
#include "windows_includes.h"

namespace dxup {
  
  namespace log {
    void internal_write(const char* prefix, const char* fmt, va_list list);

    inline void stub(const char* fmt ...) {
      if (!config::getBool(config::Log))
        return;

      #ifndef LOG_STUBS_DISABLE
      va_list list;
      va_start(list, fmt);
      internal_write("STUB", fmt, list);
      va_end(list);
      #endif
    }

    inline void warn(const char* fmt ...) {
      if (!config::getBool(config::Log))
        return;

      #ifndef LOG_STUBS_WARN
      va_list list;
      va_start(list, fmt);
      internal_write("WARN", fmt, list);
      va_end(list);
      #endif
    }

    inline void fail(const char* fmt ...) {
      if (!config::getBool(config::Log))
        return;

      #ifndef LOG_STUBS_FAIL
      va_list list;
      va_start(list, fmt);
      internal_write("FAIL", fmt, list);
      va_end(list);
      #endif
    }

    inline HRESULT d3derr(HRESULT result, const char* fmt ...) {
      if (!config::getBool(config::Log))
        return result;

      #ifndef LOG_STUBS_D3DERR
      va_list list;
      va_start(list, fmt);
      internal_write("D3DE", fmt, list);
      va_end(list);
      #endif

      return result;
    }

    inline void msg(const char* fmt ...) {
      if (!config::getBool(config::Log))
        return;

    #ifndef LOG_STUBS_MSG
      va_list list;
      va_start(list, fmt);
      internal_write("MESG", fmt, list);
      va_end(list);
    #endif
    }
  }

}