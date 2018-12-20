#include <stdint.h>

namespace dxup {

  namespace dx9asm {

    struct STATData {

      uint32_t instructionCount = 0;
      uint32_t tempRegisterCount = 0;
      uint32_t defineCount = 0;
      uint32_t declarationCount = 0;
      uint32_t floatInstructionCount = 0;
      uint32_t intInstructionCount = 0;
      uint32_t uintInstructionCount = 0;
      uint32_t staticFlowControlCount = 0;
      uint32_t dynamicFlowControlCount = 0;
      uint32_t macroInstructionCount = 0;
      uint32_t tempArrayCount = 0;
      uint32_t arrayInstructionCount = 0;
      uint32_t cutInstructionCount = 0;
      uint32_t emitInstructionCount = 0;
      uint32_t textureNormalInstructions = 0;
      uint32_t textureLoadInstructions = 0;
      uint32_t textureCompInstructions = 0;
      uint32_t textureBiasInstructions = 0;
      uint32_t textureGradientInstructions = 0;
      uint32_t movInstructionCount = 0;
      uint32_t movCInstructionCount = 0;
      uint32_t conversionInstructionCount = 0;

      uint32_t unknown1 = 0;
      uint32_t inputPrimitive = 0;
      uint32_t geometryShaderOutputTopology = 0;
      uint32_t geometryShaderMaxOutputVertexCount = 0;
      uint32_t unknown2 = 0;
      uint32_t unknown3 = 0;
      uint32_t isSampleFrequencyShader = 0;

      // DX11
      
      uint32_t unknown4 = 0;
      uint32_t controlPoints = 0;
      uint32_t hullShaderOutputPrimitive = 0;
      uint32_t hullShaderPartitioning = 0;
      uint32_t tesselatorDomain = 0;
      uint32_t barrierInstructions = 0;
      uint32_t interlockedInstructions = 0;
      uint32_t textureStoreInstructions = 0;

    };

  }

}