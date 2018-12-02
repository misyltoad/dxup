#include <stdint.h>

namespace dxapex {

  namespace dx9asm {

    struct STATData {

      uint32_t instructionCount = 0;
      uint32_t tempRegisterCount = 0;
      uint32_t defineCount = 0;
      uint32_t declCount = 0;
      uint32_t flInstCount = 0;
      uint32_t intInstCount = 0;
      uint32_t staticFlowControlCount = 0;
      uint32_t dynamicFlowControlCount = 0;
      uint32_t macroInstructionCount = 0;
      uint32_t tempArrayCount = 0;
      uint32_t arrayInstructionCount = 0;
      uint32_t cutInstructionCount = 0;
      uint32_t emitInstructionCount = 0;
      uint32_t textureNormalInstructions = 0;
      uint32_t textureLoadInstructions = 0;
      uint32_t textureComparisonInstructions = 0;
      uint32_t textureBiasInstructions = 0;
      uint32_t textureGradientInstructions = 0;
      uint32_t movInstructions = 0;
      uint32_t movCinstructions = 0;
      uint32_t conversionInstructions = 0;

      struct {
        uint32_t unknown = 0;
        uint32_t inputPrimitive = 0;
        uint32_t primitiveTopology = 0;
        uint32_t maxVertexOutputCount = 0;
        uint32_t unknown2 = 0;
        uint32_t unknown3 = 0;
      } geometryGarbage;

      uint32_t sampleFrequency = 0;

    };

  }

}