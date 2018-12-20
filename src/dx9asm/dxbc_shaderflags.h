#pragma once

namespace dxup {

  namespace dx9asm {

    namespace dxbcShaderFlags {

      const uint32_t None = 0;
      const uint32_t Debug = 1;
      const uint32_t SkipValidation = 2;
      const uint32_t SkipOptimization = 4;
      const uint32_t PackMatrixRowMajor = 8;
      const uint32_t PackMatrixColumnMajor = 16;
      const uint32_t PartialPrecision = 32;
      const uint32_t ForceVsSoftwareNoOpt = 64;
      const uint32_t ForcePsSoftwareNoOpt = 128;
      const uint32_t NoPreshader = 256;
      const uint32_t AvoidFlowControl = 512;
      const uint32_t PreferFlowControl = 1024;
      const uint32_t EnableStrictness = 2048;
      const uint32_t EnableBackwardsCompatibility = 4096;
      const uint32_t IeeeStrictness = 8192;
      const uint32_t OptimizationLevel0 = 16384;
      const uint32_t OptimizationLevel1 = 0;
      const uint32_t OptimizationLevel2 = 49152;
      const uint32_t OptimizationLevel3 = 32768;
      const uint32_t Reserved16 = 65536;
      const uint32_t WarningsAreErrors = 262144;
      const uint32_t Reserved17 = 131072;

    }

  }

}