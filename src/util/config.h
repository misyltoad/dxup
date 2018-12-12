#pragma once
#include <string>

namespace dxapex {

  namespace config {

    enum var {
      EmitNop,
      Debug,
      ShaderSpew,
      ShaderDump,
      RandomClearColour,
      UnimplementedFatal,
      Count,
    };

    float getFloat(var variable);
    int getInt(var variable);
    const std::string& getString(var variable);
    bool getBool(var variable);

  }

}