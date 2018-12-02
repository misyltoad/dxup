#pragma once
#include "dx9asm_translator.h"
#include "../util/fourcc.h"
#include "../util/misc_helpers.h"
#include "../util/shared_conversions.h"
#include "dxbc_stats.h"

namespace dxapex {

  namespace dx9asm {

    namespace chunks {
      enum {
        RDEF = 0,
        ISGN = 1,
        OSGN,
        SHDR,
        STAT,
        Count
      };
    }

    struct ChunkHeader {
      ChunkHeader(uint32_t name) : name{ name }, size{ 0 } {}

      inline void push(std::vector<uint32_t>& obj) {
        obj.push_back(name);
        obj.push_back(size);
      }

      uint32_t name;
      uint32_t size;
    };

    struct RDEFChunk {
      ChunkHeader header;

      RDEFChunk(ShaderCodeTranslator& shdrCode)
        : header{ fourcc("RDEF") } {
        
      }

      inline uint32_t getSize() {
        return 0;
      }

      inline void push(std::vector<uint32_t>& obj) {
        header.size = getSize();

        header.push(obj);
      }
    };

    template<bool Output>
    struct IOSGNChunk {
      ChunkHeader header;

      struct IOSGNElement {
        uint32_t nameOffset;
        uint32_t semanticIndex = 0;
        uint32_t systemValueType = 0;
        uint32_t componentType = 3; // <-- Floating Point 
        uint32_t registerIndex;
        uint32_t mask = 0b1111;
        uint32_t rwMask = 0b1111;
      };

      IOSGNChunk(ShaderCodeTranslator& shdrCode)
        : header{ Output ? fourcc("OSGN") : fourcc("ISGN") } {

        std::vector<RegisterMapping>& mappings = shdrCode.getRegisterMappings();
        for (const RegisterMapping& mapping : mappings) {
          if (!mapping.dclInfo.hasUsage)
            continue;

          bool valid = mapping.dx9Type == D3DSPR_INPUT && isInput() ||
                       mapping.dx9Type == D3DSPR_OUTPUT && isOutput();

          if (!valid)
            continue;

          IOSGNElement element;
          elementNames.push_back(convert::declUsage(mapping.dclInfo.usage));
          element.registerIndex = mapping.dxbcOperand.getRegNumber();
          element.semanticIndex = mapping.dclInfo.usageIndex;
          elements.push_back(element);
        }
      }

      inline bool isInput() {
        return !Output;
      }

      inline bool isOutput() {
        return Output;
      }

      inline uint32_t getSize() {
        return 0;
      }

      inline void push(std::vector<uint32_t>& obj) {
        header.size = getSize();
        elementCount = elements.size();

        header.push(obj);
        
        obj.push_back(elementCount);
        obj.push_back(magicEightBall);
      }

      uint32_t elementCount;
      uint32_t magicEightBall = 8; // Reply hazy, try again. ~ Josh

      std::vector<IOSGNElement> elements;
      std::vector<const char*> elementNames;
    };

    struct SHDRChunk {
      ChunkHeader header;

      SHDRChunk(ShaderCodeTranslator& shdrCode)
        : header{ fourcc("SHDR") } {
        versionAndType = ENCODE_D3D10_SB_TOKENIZED_PROGRAM_VERSION_TOKEN(D3D10_SB_VERTEX_SHADER, 4, 0);
        code = &shdrCode.getCode();
      }

      inline uint32_t getSize() {
        return sizeof(versionAndType) +
               sizeof(dwordCount) +
               code->size() * sizeof(uint32_t);
      }

      inline void push(std::vector<uint32_t>& obj) {
        header.size = getSize();
        dwordCount = getSize() / sizeof(uint32_t);

        header.push(obj);

        obj.push_back(versionAndType);
        obj.push_back(dwordCount);
       
        for (size_t i = 0; i < code->size(); i++)
          obj.push_back((*code)[i]);
      }

      uint32_t versionAndType;
      uint32_t dwordCount;
      const std::vector<uint32_t>* code;
    };

    struct STATChunk {
      ChunkHeader header;

      STATChunk(ShaderCodeTranslator& shdrCode)
        : header{ fourcc("STAT") } {

      }

      inline uint32_t getSize() {
        return sizeof(data);
      }

      inline void push(std::vector<uint32_t>& obj) {
        header.size = getSize();

        header.push(obj);
        pushObject(obj, data);
      }

      STATData data;
    };

  }

}