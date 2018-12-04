#pragma once
#include "../util/fourcc.h"
#include "../util/misc_helpers.h"
#include "../util/config.h"
#include "../util/shared_conversions.h"
#include "dx9asm_translator.h"
#include "dxbc_header.h"
#include "dxbc_stats.h"
#include "dxbc_shaderflags.h"
#include <string>

namespace dxapex {

  namespace dx9asm {

    struct ChunkHeader {
      ChunkHeader(uint32_t name) : name{ name }, size{ 0 } {}

      inline void push(std::vector<uint32_t>& obj) {
        obj.push_back(name);
        obj.push_back(size);
      }

      uint32_t name;
      uint32_t size;
    };

    template <uint32_t ChunkType>
    struct BaseChunk {
      BaseChunk(ShaderBytecode& bytecode) : m_bytecode{ bytecode } {}

      inline uint32_t getSize() {
        uint32_t offset = m_bytecode.getHeader()->chunkOffsets[ChunkType];
        uint32_t totalSize = m_bytecode.getByteSize();
        uint32_t chunkSizeWithHeader = totalSize - offset;
        return chunkSizeWithHeader - (uint32_t)(sizeof(ChunkHeader));
      }

      ShaderBytecode& m_bytecode;
    };

    struct RDEFChunk : public BaseChunk<chunks::RDEF> {
      ChunkHeader header;

      RDEFChunk(ShaderBytecode& bytecode)
        : BaseChunk<chunks::RDEF>{bytecode}, header{ fourcc("RDEF") } {
        
      }

      inline void push() {
        auto& obj = m_bytecode.getBytecodeVector();
        header.push(obj);

        header.size = getSize();
      }

      uint32_t constantBufferCount = 1;
      uint32_t constantBufferDescOffset = 0; // Set later
      uint32_t resourceBindingCount = 1;
      uint32_t resourceBindingDescOffset = 0; // Set later
      uint8_t minorVersion = 0;
      uint8_t majorVersion = 5;
      uint16_t programType = 0xFFEE; // Vertex Shader
      uint32_t flags = dxbcShaderFlags::NoPreshader; // NoPreshader
      uint32_t creatorOffset = 0;

      // SM5

      uint32_t rd11 = fourcc("rd11");
      uint32_t unknown1 = 60;
      uint32_t unknown2 = 24;
      uint32_t unknown3 = 32;
      uint32_t unknown4 = 40;
      uint32_t unknown5 = 36;
      uint32_t unknown6 = 12;
      uint32_t interfaceSlotCount = 0;
    };

    template<uint32_t ChunkType>
    struct IOSGNChunk : public BaseChunk<ChunkType> {
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

      IOSGNChunk(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode)
        : BaseChunk<ChunkType>{ bytecode }, header{ ChunkType == chunks::OSGN ? fourcc("OSGN") : fourcc("ISGN") } {

        std::vector<RegisterMapping>& mappings = shdrCode.getRegisterMappings();
        for (const RegisterMapping& mapping : mappings) {
          if (!mapping.dclInfo.hasUsage)
            continue;

          bool valid = mapping.dx9Type == D3DSPR_INPUT && ChunkType == chunks::ISGN ||
                       mapping.dx9Type == D3DSPR_OUTPUT && ChunkType == chunks::OSGN;

          if (!valid)
            continue;

          if (mapping.dxbcOperand.isLiteral())
            continue;

          IOSGNElement element;
          const std::string& semanticName = convert::declUsage(mapping.dclInfo.usage);
          semanticNames.push_back(&semanticName);
          element.registerIndex = mapping.dxbcOperand.getRegNumber();
          element.semanticIndex = mapping.dclInfo.usageIndex;
          elements.push_back(element);
        }
      }

      inline void push() {
        auto& obj = m_bytecode.getBytecodeVector();
        elementCount = elements.size();

        header.push(obj);
        
        obj.push_back(elementCount);
        obj.push_back(magicEightBall);

        IOSGNElement* elementStart = (IOSGNElement*) nextPtr(obj);

        for (IOSGNElement& element : elements)
          pushObject(obj, element);

        for (size_t i = 0; i < semanticNames.size(); i++) {
          elementStart[i].nameOffset = getSize();
          pushAlignedString(obj, *semanticNames[i]);
        }

        header.size = getSize();
      }

      uint32_t elementCount;
      uint32_t magicEightBall = 8; // Reply hazy, try again. ~ Josh

      std::vector<IOSGNElement> elements;
      std::vector<const std::string*> semanticNames;
    };

    struct SHEXChunk : public BaseChunk<chunks::SHEX> {
      ChunkHeader header;

      SHEXChunk(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode)
        : BaseChunk<chunks::SHEX>{ bytecode }, header{ fourcc("SHEX") } {
        code = &shdrCode.getCode();
      }

      inline void push() {
        auto& obj = m_bytecode.getBytecodeVector();
        header.push(obj);

        obj.push_back(versionAndType);
        obj.push_back(dwordCount);
        uint32_t* dwordCountPtr = lastPtr(obj);
       
        for (size_t i = 0; i < code->size(); i++)
          obj.push_back((*code)[i]);

        header.size = getSize();
        *dwordCountPtr = header.size / sizeof(uint32_t);
      }

      uint32_t versionAndType = ENCODE_D3D10_SB_TOKENIZED_PROGRAM_VERSION_TOKEN(D3D10_SB_VERTEX_SHADER, 5, 0);
      uint32_t dwordCount = 0;
      const std::vector<uint32_t>* code;
    };

    struct STATChunk : public BaseChunk<chunks::STAT> {
      ChunkHeader header;

      STATChunk(ShaderBytecode& bytecode)
        : BaseChunk<chunks::STAT>{ bytecode }, header{ fourcc("STAT") } {

      }

      inline void push() {
        auto& obj = m_bytecode.getBytecodeVector();
        header.push(obj);
        pushObject(obj, data);

        header.size = getSize();
      }

      STATData data;
    };

  }

}