#pragma once
#include "dx9asm_translator.h"
#include "../util/fourcc.h"
#include "../util/misc_helpers.h"
#include "../util/config.h"
#include "../util/shared_conversions.h"
#include "dxbc_stats.h"
#include <string>

namespace dxapex {

  namespace dx9asm {

    namespace chunks {
      enum {
        RDEF = 0,
        ISGN = 1,
        OSGN,
        SHEX,
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

    struct Chunk {
      inline uint32_t getSize(std::vector<uint32_t>& obj) {
        return obj.size() - sizeof(ChunkHeader);
      }
    };

    struct RDEFChunk : public Chunk {
      ChunkHeader header;

      RDEFChunk(ShaderCodeTranslator& shdrCode)
        : header{ fourcc("RDEF") } {
        
      }

      inline void push(std::vector<uint32_t>& obj) {
        header.push(obj);

        header.size = getSize(obj);
      }
    };

    template<bool Output>
    struct IOSGNChunk : public Chunk {
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
          const std::string& semanticName = convert::declUsage(mapping.dclInfo.usage);
          semanticNames.push_back(&semanticName);
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

      inline void push(std::vector<uint32_t>& obj) {
        elementCount = elements.size();

        header.push(obj);
        
        obj.push_back(elementCount);
        obj.push_back(magicEightBall);

        IOSGNElement* elementStart = (IOSGNElement*) nextPtr(obj);

        for (IOSGNElement& element : elements)
          pushObject(obj, element);

        for (size_t i = 0; i < semanticNames.size(); i++) {
          elementStart[i].nameOffset = getSize(obj);
          pushAlignedString(obj, *semanticNames[i]);
        }

        header.size = getSize(obj);
      }

      uint32_t elementCount;
      uint32_t magicEightBall = 8; // Reply hazy, try again. ~ Josh

      std::vector<IOSGNElement> elements;
      std::vector<const std::string*> semanticNames;
    };

    struct SHEXChunk : public Chunk {
      ChunkHeader header;

      SHEXChunk(ShaderCodeTranslator& shdrCode)
        : header{ fourcc("SHEX") } {
        code = &shdrCode.getCode();
      }

      inline void push(std::vector<uint32_t>& obj) {

        header.push(obj);

        obj.push_back(versionAndType);
        obj.push_back(dwordCount);
        uint32_t* dwordCountPtr = lastPtr(obj);
       
        for (size_t i = 0; i < code->size(); i++)
          obj.push_back((*code)[i]);

        header.size = getSize(obj);
        *dwordCountPtr = header.size / sizeof(uint32_t);
      }

      uint32_t versionAndType = ENCODE_D3D10_SB_TOKENIZED_PROGRAM_VERSION_TOKEN(D3D10_SB_VERTEX_SHADER, 5, 0);
      uint32_t dwordCount = 0;
      const std::vector<uint32_t>* code;
    };

    struct STATChunk : public Chunk {
      ChunkHeader header;

      STATChunk(ShaderCodeTranslator& shdrCode)
        : header{ fourcc("STAT") } {

      }

      inline void push(std::vector<uint32_t>& obj) {
        header.push(obj);
        pushObject(obj, data);

        header.size = getSize(obj);
      }

      STATData data;
    };

  }

}