#pragma once
#include <string>

namespace dxup {

  namespace config {

    enum var {
      EmitNop,
      Log,
      ShaderModel,
      Debug,
      ShaderSpew,
      ShaderDump,
      RandomClearColour,
      UnimplementedFatal,
      InitialHideCursor,
      RefactoringAllowed,
      GDICompatible,
      Count,
    };

    float getFloat(var variable);
    int getInt(var variable);
    const std::string& getString(var variable);
    bool getBool(var variable);

  }

}