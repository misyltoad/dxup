#include "config.h"
#include "log.h"
#include <stdio.h>
#include <map>
#include <array>

namespace dxup { 

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
		  initVar(var::EmitNop, "DXUP_EMITNOP", "0");
          initVar(var::Log, "DXUP_LOG", "1");
          initVar(var::ShaderModel, "DXUP_SHADERMODEL", "2b");

#ifdef _DEBUG
          initVar(var::Debug, "DXUP_DEBUG", "1");
#else
          initVar(var::Debug, "DXUP_DEBUG", "0");
#endif
          initVar(var::RandomClearColour, "DXUP_RANDOMCLEARCOLOUR", "0");
          initVar(var::ShaderSpew, "DXUP_SHADERSPEW", "0");
          initVar(var::ShaderDump, "DXUP_SHADERDUMP", "0");
          initVar(var::UnimplementedFatal, "DXUP_UNIMPLEMENTEDFATAL", "0");
          initVar(var::InitialHideCursor, "DXUP_INITIALHIDECURSOR", "0");
          initVar(var::RefactoringAllowed, "DXUP_REFACTORINGALLOWED", "1");
          initVar(var::GDICompatible, "DXUP_GDI_COMPATIBLE", "0");
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

    ConfigMgr dxupConfig;

    float getFloat(var variable) {
      return dxupConfig.getFloat(variable);
    }

    int getInt(var variable) {
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
