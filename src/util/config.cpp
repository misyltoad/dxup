#include "config.h"
#include "log.h"
#include <stdio.h>
#include <map>
#include <array>
#include <cstdlib>

namespace dxup { 

  namespace config {

    struct VarValue {
      std::string strVal;
      float floatVal;
      int64_t intVal;
      bool boolVal;
    };

    inline bool IsWine() {
      static bool cachedWine = false;
      static bool isWine = false;

      if (cachedWine == false) {
        typedef const char*(*WineGetVersion)(void);
        WineGetVersion wine_get_version;

        HMODULE module = GetModuleHandleA("ntdll.dll");

        wine_get_version = (WineGetVersion)GetProcAddress(module, "wine_get_version");
        if (wine_get_version)
          isWine = true;
        cachedWine = true;
      }

      return isWine;
    }

    class ConfigMgr {
      
    public:

      ConfigMgr() {
		  initVar(var::EmitNop, "DXUP_EMITNOP", "0");
          initVar(var::Log, "DXUP_LOG", "1");
          initVar(var::ShaderModel, "DXUP_SHADERMODEL", "2b");

#ifdef _DEBUG
          initVar(var::Debug, "DXUP_DEBUG", "1");
#else
          initVar(var::Debug, "DXUP_DEBUG", "0");
#endif
          if (IsWine())
            initVar(var::Wine, "DXUP_WINE", "1");
          else
            initVar(var::Wine, "DXUP_WINE", "0");

          initVar(var::ForceWindowed, "DXUP_FORCEWINDOWED", "0");
          initVar(var::RandomClearColour, "DXUP_RANDOMCLEARCOLOUR", "0");
          initVar(var::ShaderSpew, "DXUP_SHADERSPEW", "0");
          initVar(var::ShaderDump, "DXUP_SHADERDUMP", "0");
          initVar(var::UnimplementedFatal, "DXUP_UNIMPLEMENTEDFATAL", "0");
          initVar(var::InitialHideCursor, "DXUP_INITIALHIDECURSOR", "0");
          initVar(var::RefactoringAllowed, "DXUP_REFACTORINGALLOWED", "1");
          initVar(var::GDICompatible, "DXUP_GDI_COMPATIBLE", "0");
          initVar(var::RespectPrecision, "DXUP_RESPECT_PRECISION", "1");

          initVar(var::RespectVSync, "DXUP_RESPECT_VSYNC", "1");
          initVar(var::UseFakes, "DXUP_USEFAKES", "1");
          initVar(var::FakeDriver, "DXUP_FAKE_DRIVER", "aticfx32.dll");
          initVar(var::FakeDescription, "DXUP_FAKE_DESCRIPTION", "AMD Radeon R9 200 Series");
          initVar(var::FakeDeviceName, "DXUP_FAKE_DEVICENAME", "\\\\.\\DISPLAY1");
          initVar(var::FakeDriverVersion, "DXUP_FAKE_DRIVERVERSION", "7036961300353938");
          initVar(var::FakeVendorId, "DXUP_FAKE_VENDORID", "4098");
          initVar(var::FakeDeviceId, "DXUP_FAKE_DEVICEID", "26520");
          initVar(var::FakeSubSysId, "DXUP_FAKE_SUBSYSID", "661984354");
          initVar(var::FakeRevision, "DXUP_FAKE_REVISION", "0");
          initVar(var::FakeDeviceIdentifier, "DXUP_FAKE_DEVICEIDENTIFIER", "{D7B71EE2-24D8-11CF-4064-EE3DBBC2D435}");
      }

	  void initVar(var variable, const char* name, const char* defaultValue) {
        const char* value = getenv(name);
        const char* logending = "";

        if (value == nullptr) {
	      logending = " [default]";
	      value = defaultValue;
        }

        value = value != nullptr ? value : defaultValue;

        log::msg("%s=%s%s", name, value, logending);

        VarValue typedValues;
        typedValues.strVal = value;
        typedValues.intVal = std::atoll(value);
        typedValues.boolVal = typedValues.intVal != 0;
        typedValues.floatVal = atof(value);

        m_varList[variable] = typedValues;
	  }

      float getFloat(var variable) {
        return m_varList[variable].floatVal;
      }

      int64_t getInt(var variable) {
		  return m_varList[variable].intVal;
	  }

      const std::string& getString(var variable) {
        return m_varList[variable].strVal;
      }

      bool getBool(var variable) {
        return m_varList[variable].boolVal;
      }

    private:

      std::array<VarValue, var::Count> m_varList;

    };

    ConfigMgr dxupConfig;

    float getFloat(var variable) {
      return dxupConfig.getFloat(variable);
    }

    int64_t getInt(var variable) {
      return dxupConfig.getInt(variable);
    }

    const std::string& getString(var variable) {
      return dxupConfig.getString(variable);
    }

    bool getBool(var variable) {
      return dxupConfig.getBool(variable);
    }

  }

}
