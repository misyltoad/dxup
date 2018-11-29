#include "config.h"
#include "log.h"
#include <stdio.h>
#include <map>
#include <array>

namespace dxapex { 

  namespace config {

    struct VarValue {
      std::string strVal;
      float floatVal;
	  int intVal;
      bool boolVal;
    };

    class ConfigMgr {
      
    public:

      ConfigMgr() {
		  initVar(var::EmitNop, "DXAPEX_EMITNOP", "0");
          initVar(var::DebugDevice, "DXAPEX_DEBUGDEVICE", "0");
          initVar(var::ShaderSpew, "DXAPEX_SHADERSPEW", "0");
      }

	  void initVar(var variable, const char* name, const char* default) {
        const char* value = getenv(name);
        const char* logending = "";

        if (value == nullptr) {
	      logending = " [default]";
	      value = default;
        }

        value = value != nullptr ? value : default;

        log::msg("%s=%s%s", name, value, logending);

        VarValue typedValues;
        typedValues.strVal = value;
        typedValues.intVal = atoi(value);
        typedValues.boolVal = typedValues.intVal != 0;
        typedValues.floatVal = atof(value);

        m_varList[variable] = typedValues;
	  }

      float getFloat(var variable) {
        return m_varList[variable].floatVal;
      }

	  int getInt(var variable) {
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

    ConfigMgr dxapexConfig;

    float getFloat(var variable) {
      return dxapexConfig.getFloat(variable);
    }

    int getInt(var variable) {
      return dxapexConfig.getInt(variable);
    }

    const std::string& getString(var variable) {
      return dxapexConfig.getString(variable);
    }

    bool getBool(var variable) {
      return dxapexConfig.getBool(variable);
    }

  }

}
