#include "dxbc_chunks.h"
#include "dxbc_shaderflags.h"
#include "../util/placeholder_ptr.h"

namespace dxapex {

  namespace dx9asm {

    template <uint32_t ChunkType>
    class BaseChunk {

    public:
      inline uint32_t getChunkSize(ShaderBytecode& bytecode) {
        uint32_t offset = bytecode.getHeader()->chunkOffsets[ChunkType];
        uint32_t totalSize = bytecode.getByteSize();
        uint32_t chunkSizeWithHeader = totalSize - offset;
        return chunkSizeWithHeader - (uint32_t)(sizeof(ChunkHeader));
      }

      inline void push(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) {
        pushInternal(bytecode, shdrCode);

        if (getChunkSize(bytecode) % sizeof(uint32_t))
          log::warn("Chunk not dword aligned!");
      }

    private:
      virtual void pushInternal(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) = 0;

    };

    class RDEFChunk : public BaseChunk<chunks::RDEF> {
      #pragma pack(1)
      struct VariableInfo {
        uint32_t nameOffset;
        uint32_t startOffset;
        uint32_t size = 4 * sizeof(float);
        uint32_t flags = D3D_SVF_USED;
        uint32_t typeOffset;
        uint32_t defaultValueOffset;

        // DX11
        uint32_t startTexture = 0;
        uint32_t textureSize = 0;
        uint32_t startSampler = 0;
        uint32_t samplerSize = 0;
      };

      template <typename T>
      void forEachVariable(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode, T func) {
        uint32_t i = 0;
        for (const RegisterMapping& mapping : shdrCode.getRegisterMappings()) {
          if (mapping.dxbcOperand.getRegisterType() != D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER)
            continue;

          func(mapping, i);
          i++;
        }
      }

      void pushInternal(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) override {
        auto& obj = bytecode.getBytecodeVector();

        PlaceholderPtr<uint32_t> headerChunkSize{ "[RDEF] Chunk Header - Chunk Data Size", &((ChunkHeader*)nextPtr(obj))->size };
        ChunkHeader{ fourcc("RDEF") }.push(obj); // [PUSH] Chunk Header

        uint32_t cbufferVariableCount = 0;
        forEachVariable(bytecode, shdrCode, [&](const RegisterMapping& mapping, uint32_t i) {
          cbufferVariableCount++;
        });
        uint32_t constantBufferCount = cbufferVariableCount == 0 ? 0 : 1;

        obj.push_back(constantBufferCount); // [PUSH] Constant Buffer Count
        PlaceholderPtr<uint32_t> constantBufferDescOffset{ "[RDEF] Constant Buffer Desc Offset", nextPtr(obj) };
        obj.push_back(0); // [PUSH] Constant Buffer Desc Offset
        obj.push_back(1); // [PUSH] Resource Binding Count
        PlaceholderPtr<uint32_t>  resourceBindingDescOffset{ "[RDEF] Resource Binding Desc Offset", nextPtr(obj) };
        obj.push_back(0); // [PUSH] Resource Binding Desc Offset

        uint32_t versionInfo = 0xFFFE << 16; // Vertex Shader
        versionInfo |= 5 << 8; // Major Version
        versionInfo |= 0 << 0; // Minor Version
        obj.push_back(versionInfo); // [PUSH] Version Info (u8 minor, u8 major, u16 type)

        obj.push_back(dxbcShaderFlags::NoPreshader); // [PUSH] Shader Flags
        PlaceholderPtr<uint32_t> creatorOffset{ "[RDEF] Creator Offset", nextPtr(obj) };
        obj.push_back(0); // [PUSH] Creator Offset

        //if (SM5) {

        obj.push_back(fourcc("RD11")); // [PUSH] RD11
        obj.push_back(60); // [PUSH] Unknown 1
        obj.push_back(24); // [PUSH] Unknown 2
        obj.push_back(32); // [PUSH] Unknown 3
        obj.push_back(40); // [PUSH] Unknown 4
        obj.push_back(36); // [PUSH] Unknown 5
        obj.push_back(12); // [PUSH] Unknown 6
        obj.push_back(0); // [PUSH] Interface Slot Count

        //}
        
        // Just one for now...
        constantBufferDescOffset = getChunkSize(bytecode);
        if (constantBufferCount != 0) {
          // Constant Buffer
          PlaceholderPtr<uint32_t> cbufNameOffset{ "[RDEF] Constant Buffer Variable Count", nextPtr(obj) };
          obj.push_back(0); // [PUSH] CBuf Name Offset

          obj.push_back(cbufferVariableCount); // [PUSH] CBuf Variable Count

          PlaceholderPtr<uint32_t> cbufVariableOffset{ "[RDEF] Constant Buffer Variable Offset", nextPtr(obj) };
          obj.push_back(0); // [PUSH] Constant Buffer Variable Offset

          obj.push_back(cbufferVariableCount * sizeof(float)); // [PUSH] Constant Buffer Size (bytes)
          obj.push_back(0); // [PUSH] Constant Buffer Flags (None)
          obj.push_back(0); // [PUSH] Constant Buffer Type (CBuffer)

          cbufVariableOffset = getChunkSize(bytecode);

          VariableInfo* variables = (VariableInfo*)nextPtr(obj);
          forEachVariable(bytecode, shdrCode, [&](const RegisterMapping& mapping, uint32_t i) {
            VariableInfo info;
            info.startOffset = i * 4 * sizeof(float);
            pushObject(obj, info);
          });

          struct {
            float values[4] = { 0 };
          } defaultValues;
          uint32_t defaultValueOffset = getChunkSize(bytecode);
          pushObject(obj, defaultValues); // [PUSH] Default Value for our Constants

          // Variable Type (they're all the same for now, no bool yet)
          #pragma pack(1)
          struct {
            uint16_t varClass = D3D_SVC_VECTOR;
            uint16_t varType = D3D_SVT_FLOAT;
            uint16_t rows = 1;
            uint16_t columns = 4;
            uint16_t arrayCount = 0;
            uint16_t structureCount = 0;
            uint32_t byteOffsetToFirst = 0;

            // DX11

            uint32_t parentTypeOffset = 0;
            uint32_t unknown1 = 0;
            uint32_t unknown2 = 0;
            uint32_t unknown3 = 0;
            uint32_t parentNameOffset = 0;
          } variableType;
          uint32_t variableTypeOffset = getChunkSize(bytecode);
          pushObject(obj, variableType);

          // Resource Binding
          #pragma pack(1)
          struct ResourceBinding {
            uint32_t nameOffset;
            uint32_t bindingType = 0; // cbuffer
            uint32_t returnType = 0; // na
            uint32_t rvd = 0; // na
            uint32_t samples = 0;
            uint32_t bindPoint = 0;
            uint32_t bindCount = 1;
            uint32_t flags = 0;
          } resourceBinding;
          ResourceBinding* binding = (ResourceBinding*)nextPtr(obj);
          resourceBindingDescOffset = getChunkSize(bytecode);
          pushObject(obj, resourceBinding);

          // Push and Fixup Strings
          binding->nameOffset = getChunkSize(bytecode);
          pushAlignedString(obj, "dx9_constant_buffer");

          forEachVariable(bytecode, shdrCode, [&](const RegisterMapping& mapping, uint32_t i) {
            VariableInfo& info = variables[i];
            info.defaultValueOffset = defaultValueOffset;
            info.typeOffset = variableTypeOffset;

            char name[6];
            snprintf(name, 6, "c%d", i);
            info.nameOffset = getChunkSize(bytecode);
            pushAlignedString(obj, name, strlen(name));
          });

          cbufNameOffset = getChunkSize(bytecode);
          pushAlignedString(obj, "dx9_mapped_constants");
        }

        creatorOffset = getChunkSize(bytecode);
        pushAlignedString(obj, ":frog:"); // [PUSH] Creator (ribbit.)

        headerChunkSize = getChunkSize(bytecode);
      }
    };

    class SHEXChunk : public BaseChunk<chunks::SHEX> {
      void writeDcls(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) {
        auto& obj = bytecode.getBytecodeVector();

        // Temps
        {
          DXBCOperation{ D3D10_SB_OPCODE_DCL_TEMPS, false, 2 }.push(obj);
          obj.push_back(shdrCode.getHighestIdForDXBCType(D3D10_SB_OPERAND_TYPE_TEMP) + 1); // Followed by DWORD count of temps. Not an operand!
        }
      }

      void pushInternal(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) override {
        auto& obj = bytecode.getBytecodeVector();

        PlaceholderPtr<uint32_t> headerChunkSize{ "[SHEX] Chunk Header - Chunk Data Size", &((ChunkHeader*)nextPtr(obj))->size };
        ChunkHeader{ fourcc("SHEX") }.push(obj); // [PUSH] Chunk Header
        
        obj.push_back(ENCODE_D3D10_SB_TOKENIZED_PROGRAM_VERSION_TOKEN(D3D10_SB_VERTEX_SHADER, 5, 0)); // [PUSH] DXBC Version Token - VerTok

        PlaceholderPtr<uint32_t> dwordCount{ "[SHEX] Dword Count", nextPtr(obj) };
        obj.push_back(0); // [PUSH] Dword Count

        writeDcls(bytecode, shdrCode);

        for (uint32_t token : shdrCode.getCode())
          obj.push_back(token);

        headerChunkSize = getChunkSize(bytecode);
        dwordCount = getChunkSize(bytecode) / sizeof(uint32_t);
      }
    };

    class STATChunk : public BaseChunk<chunks::STAT> {
      void pushInternal(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) override {
        auto& obj = bytecode.getBytecodeVector();

        PlaceholderPtr<uint32_t> headerChunkSize{ "[STAT] Chunk Header - Chunk Data Size", &((ChunkHeader*)nextPtr(obj))->size };
        ChunkHeader{ fourcc("STAT") }.push(obj); // [PUSH] Chunk Header
        pushObject(obj, STATData{}); // [PUSH] Dummy Stat Data
        uint32_t size = sizeof(STATData);

        headerChunkSize = getChunkSize(bytecode);
      }
    };

    template <uint32_t ChunkType>
    class IOSGNChunk : public BaseChunk<ChunkType> {

      // Pack properly for DXBC formatting...
      struct IOSGNElement {
        IOSGNElement() {
          mask |= 0b1111;
          mask |= 0b1111 << 8;
        }

        uint32_t nameOffset;
        uint32_t semanticIndex;
        uint32_t systemValueType = 0;
        uint32_t componentType = D3D_SVT_FLOAT;
        uint32_t registerIndex;
        uint32_t mask = 0;
      };

      template <typename T>
      void forEachValidElement(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode, T func) {
        uint32_t i = 0;
        for (const RegisterMapping& mapping : shdrCode.getRegisterMappings()) {
          if (!mapping.dclInfo.hasUsage)
            continue;

          bool valid = mapping.dx9Type == D3DSPR_INPUT && ChunkType == chunks::ISGN ||
                       mapping.dx9Type == D3DSPR_OUTPUT && ChunkType == chunks::OSGN;

          if (!valid)
            continue;

          if (mapping.dxbcOperand.isLiteral()) {
            log::warn("IO is a literal..?");
            continue;
          }

          func(mapping, i);
          i++;
        }
      }

      void pushInternal(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) override {
        auto& obj = bytecode.getBytecodeVector();

        PlaceholderPtr<uint32_t> headerChunkSize{ "[IOSGN] Chunk Header - Chunk Data Size", &((ChunkHeader*)nextPtr(obj))->size };
        ChunkHeader{ fourcc(ChunkType == chunks::ISGN ? "ISGN" : "OSGN") }.push(obj); // [PUSH] Chunk Header

        PlaceholderPtr<uint32_t> elementCount{ "[ISOGN] Element Count", nextPtr(obj) };
        obj.push_back(0); // [PUSH] Element Count
        obj.push_back(8); // [PUSH] Magic 8-Ball Constant ~ Reply hazy, try again.

        IOSGNElement* elementStart = (IOSGNElement*)nextPtr(obj);

        forEachValidElement(bytecode, shdrCode, [&](const RegisterMapping& mapping, uint32_t i) {
          IOSGNElement element;
          element.nameOffset = 0; // <-- Must be set later!
          element.registerIndex = mapping.dxbcOperand.getRegNumber();
          element.semanticIndex = mapping.dclInfo.usageIndex;
          pushObject(obj, element);
        });

        uint32_t count = 0;
        forEachValidElement(bytecode, shdrCode, [&](const RegisterMapping& mapping, uint32_t i) {
          elementStart[i].nameOffset = getChunkSize(bytecode);
          pushAlignedString(obj, convert::declUsage(mapping.dclInfo.usage));

          count++;
        });

        elementCount = count;

        headerChunkSize = getChunkSize(bytecode);
      }
    };

    void writeRDEF(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) {
      RDEFChunk{}.push(bytecode, shdrCode);
    }

    void writeSHEX(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) {
      SHEXChunk{}.push(bytecode, shdrCode);
    }

    void writeSTAT(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) {
      STATChunk{}.push(bytecode, shdrCode);
    }

    void writeISGN(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) {
      IOSGNChunk<chunks::ISGN>{}.push(bytecode, shdrCode);
    }

    void writeOSGN(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) {
      IOSGNChunk<chunks::OSGN>{}.push(bytecode, shdrCode);
    }
  }

}