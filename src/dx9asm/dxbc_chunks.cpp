#include "dxbc_chunks.h"
#include "dxbc_shaderflags.h"
#include "../util/placeholder_ptr.h"
#include "dx9asm_modifiers.h"

namespace dxup {

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

        if (this->getChunkSize(bytecode) % sizeof(uint32_t))
          log::warn("Chunk not dword aligned!");
      }

    private:
      virtual void pushInternal(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) = 0;

    };

    template <typename T>
    void forEachVariable(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode, T func) {
      uint32_t num = 0;
      if (shdrCode.isIndirectMarked())
        num = 256 + 16 + 16; // Do all 256 if we use indirect addressing.
      else
        num = shdrCode.getRegisterMap().getDXBCTypeCount(D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER);

      for (uint32_t i = 0; i < num; i++) {
        bool used = false;
        for (const RegisterMapping& mapping : shdrCode.getRegisterMap().getRegisterMappings()) {
          if (mapping.dxbcOperand.getRegisterType() == D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER && mapping.dxbcOperand.getRegNumber() == i)
            used = true;
        }

        func(i, used);
      }
    }

    constexpr bool isInput(uint32_t ChunkType) {
      return ChunkType == chunks::ISGN;
    }

    template <bool Input, typename T>
    void forEachValidElement(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode, T func) {
      uint32_t i = 0;
      for (const RegisterMapping& mapping : shdrCode.getRegisterMap().getRegisterMappings()) {
        bool valid = (mapping.dclInfo.type == UsageType::Input && Input) ||
                     (mapping.dclInfo.type == UsageType::Output && !Input);

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

    class RDEFChunk : public BaseChunk<chunks::RDEF> {
      #pragma pack(1)
      struct VariableInfo {
        uint32_t nameOffset;
        uint32_t startOffset;
        uint32_t size;
        uint32_t flags;
        uint32_t typeOffset;
        uint32_t defaultValueOffset;

        // DX11
        uint32_t startTexture = 0xFFFFFFFFu;
        uint32_t textureSize = 0;
        uint32_t startSampler = 0xFFFFFFFFu;
        uint32_t samplerSize = 0;
      };

      #pragma pack(1)
      struct ResourceBinding {
        uint32_t nameOffset = 0;
        uint32_t bindingType = 0; // cbuffer
        uint32_t returnType = 0; // na
        uint32_t rvd = 0; // na
        uint32_t samples = 0;
        uint32_t bindPoint = 0;
        uint32_t bindCount = 1;
        uint32_t flags = 0;
      };

      void pushInternal(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) override {
        auto& obj = bytecode.getBytecodeVector();

        PlaceholderPtr<uint32_t> headerChunkSize{ "[RDEF] Chunk Header - Chunk Data Size", &((ChunkHeader*)nextPtr(obj))->size };
        ChunkHeader{ fourcc("RDEF") }.push(obj); // [PUSH] Chunk Header

        uint32_t cbufferVariableCount = 0;
        forEachVariable(bytecode, shdrCode, [&](uint32_t i, bool used) {
          cbufferVariableCount++;
        });
        uint32_t constantBufferCount = cbufferVariableCount == 0 ? 0 : 1;

        obj.push_back(constantBufferCount); // [PUSH] Constant Buffer Count
        PlaceholderPtr<uint32_t> constantBufferDescOffset{ "[RDEF] Constant Buffer Desc Offset", nextPtr(obj) };
        obj.push_back(0); // [PUSH] Constant Buffer Desc Offset
        PlaceholderPtr<uint32_t> resourceBindingCount{ "[RDEF] Resource Binding Count", nextPtr(obj) };
        obj.push_back(0); // [PUSH] Resource Binding Count
        PlaceholderPtr<uint32_t>  resourceBindingDescOffset{ "[RDEF] Resource Binding Desc Offset", nextPtr(obj) };
        obj.push_back(0); // [PUSH] Resource Binding Desc Offset

        uint32_t versionInfo = shdrCode.getShaderType() == ShaderType::Vertex ? (0xFFFE << 16) : (0xFFFF << 16);
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

        ResourceBinding* constantBinding = nullptr;
        uint32_t bindingCount = 0;
        FixedBuffer<16, ResourceBinding*> samplerBindings;
        FixedBuffer<16, ResourceBinding*> textureBindings;

        if (constantBufferCount != 0 || shdrCode.isAnySamplerUsed()) {
          resourceBindingDescOffset = this->getChunkSize(bytecode);

          if (shdrCode.isAnySamplerUsed()) {
            for (uint32_t i = 0; i < 16; i++) {
              if (shdrCode.isSamplerUsed(i)) {
                SamplerDesc* sampler = shdrCode.getSampler(i);
                samplerBindings.push_back((ResourceBinding*)nextPtr(obj));
                {
                  ResourceBinding binding{};
                  binding.bindingType = 3; // sampler
                  binding.bindPoint = i;
                  binding.samples = 1;
                  pushObject(obj, binding);
                  bindingCount++;
                }
                textureBindings.push_back((ResourceBinding*)nextPtr(obj));
                {
                  ResourceBinding binding{};
                  binding.bindingType = 2; // tex
                  binding.bindPoint = i;
                  binding.returnType = 5; // float

                  if (sampler->dimension == D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D)
                    binding.rvd = 4;
                  else if (sampler->dimension == D3D10_SB_RESOURCE_DIMENSION_TEXTURE3D)
                    binding.rvd = 8;
                  else if (sampler->dimension == D3D10_SB_RESOURCE_DIMENSION_TEXTURECUBE)
                    binding.rvd = 9;

                  binding.flags |= 0xc;
                  pushObject(obj, binding);
                  bindingCount++;
                }
              }
            }
          }

          if (constantBufferCount != 0) {
            // Resource Binding
            constantBinding = (ResourceBinding*)nextPtr(obj);
            ResourceBinding blankBinding;
            pushObject(obj, blankBinding);
            bindingCount++;
          }

        }
        else
          resourceBindingDescOffset = 0;

        resourceBindingCount = bindingCount;
        
        // Just one for now...
        constantBufferDescOffset = this->getChunkSize(bytecode);
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

          cbufVariableOffset = this->getChunkSize(bytecode);

          VariableInfo* variables = (VariableInfo*)nextPtr(obj);
          forEachVariable(bytecode, shdrCode, [&](uint32_t i, bool used) {
            VariableInfo info;
            info.flags = used ? D3D_SVF_USED : 0;
            info.startOffset = i * 4 * sizeof(float);
            pushObject(obj, info);
          });

          //struct {
          //  float values[4] = { 0 };
          //} defaultValues;
          //uint32_t defaultValueOffset = this->getChunkSize(bytecode);
          //pushObject(obj, defaultValues); // [PUSH] Default Value for our Constants

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
          uint32_t floatTypeOffset = this->getChunkSize(bytecode);
          pushObject(obj, variableType);

          variableType.varClass = D3D_SVC_SCALAR;
          variableType.varType = D3D_SVT_INT;

          uint32_t intVecTypeOffset = this->getChunkSize(bytecode);
          pushObject(obj, variableType);

          //variableType.columns = 1;

          uint32_t boolTypeOffset = this->getChunkSize(bytecode);
          pushObject(obj, variableType);

          // Push and Fixup Strings
          constantBinding->nameOffset = this->getChunkSize(bytecode);
          pushAlignedString(obj, "dx9_constant_buffer");

          forEachVariable(bytecode, shdrCode, [&](uint32_t i, bool used) {
            VariableInfo& info = variables[i];
            //info.defaultValueOffset = defaultValueOffset;
            info.defaultValueOffset = 0;

            if (i < 256) {
              // float constants
              info.size = 4 * sizeof(float);
              info.typeOffset = floatTypeOffset;
            }
            else if (i < 256 + 16) {
              // int constants
              info.size = 4 * sizeof(int);
              info.typeOffset = intVecTypeOffset;
            }
            else {
              // bool constants
              info.size = 4 * sizeof(int); //sizeof(int);
              info.typeOffset = boolTypeOffset;
            }

            char name[6];
            snprintf(name, 6, "c%d", i);
            info.nameOffset = this->getChunkSize(bytecode);
            pushAlignedString(obj, name, strlen(name));
          });

          cbufNameOffset = this->getChunkSize(bytecode);
          pushAlignedString(obj, "dx9_mapped_constants");
        }

        for (size_t i = 0; i < samplerBindings.size(); i++) {
          ResourceBinding* binding = samplerBindings.get(i);

          binding->nameOffset = this->getChunkSize(bytecode);
          char name[64] = "";
          snprintf(name, 64, "sampler%d", binding->bindPoint);
          pushAlignedString(obj, name);
        }

        for (size_t i = 0; i < textureBindings.size(); i++) {
          ResourceBinding* binding = textureBindings.get(i);

          binding->nameOffset = this->getChunkSize(bytecode);
          char name[64] = "";
          snprintf(name, 64, "texture%d", binding->bindPoint);
          pushAlignedString(obj, name);
        }

        creatorOffset = this->getChunkSize(bytecode);
        pushAlignedString(obj, ":frog:"); // [PUSH] Creator (ribbit.)

        headerChunkSize = this->getChunkSize(bytecode);
      }
    };

    class SHEXChunk : public BaseChunk<chunks::SHEX> {

      template <bool Input>
      void writeIODcls(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) {
        auto& obj = bytecode.getBytecodeVector();
        forEachValidElement<Input>(bytecode, shdrCode, [&](const RegisterMapping& mapping, uint32_t i) {
          uint32_t siv = convert::sysValue(Input && shdrCode.getShaderType() == ShaderType::Vertex, mapping.dclInfo.target, mapping.dclInfo.usage);
          const bool hasSiv = siv != D3D_NAME_UNDEFINED;

          uint32_t opcode = 0;
          uint32_t interpMode = UINT32_MAX;
          
          if (shdrCode.getShaderType() == ShaderType::Vertex) {
            if (Input)
              opcode = hasSiv ? D3D10_SB_OPCODE_DCL_INPUT_SIV : D3D10_SB_OPCODE_DCL_INPUT;
            else
              opcode = hasSiv ? D3D10_SB_OPCODE_DCL_OUTPUT_SIV : D3D10_SB_OPCODE_DCL_OUTPUT;
          }
          else {
            if (Input) {
              opcode = hasSiv ? D3D10_SB_OPCODE_DCL_INPUT_PS_SIV : D3D10_SB_OPCODE_DCL_INPUT_PS;
              interpMode = mapping.dclInfo.centroid ? D3D10_SB_INTERPOLATION_LINEAR_CENTROID : D3D10_SB_INTERPOLATION_LINEAR;
            }
            else
              opcode = hasSiv ? D3D10_SB_OPCODE_DCL_OUTPUT_SIV : D3D10_SB_OPCODE_DCL_OUTPUT;
          }

          uint32_t lengthOffset = hasSiv ? 1 : 0;

          DXBCOperand operand = mapping.dxbcOperand;
          
          if (!Input)
            operand.setSwizzleOrWritemask(mapping.writeMask);
          else
            operand.setSwizzleOrWritemask(mapping.readMask);


          DXBCOperation{ opcode, false, UINT32_MAX, lengthOffset, interpMode }
            .appendOperand(operand)
            .push(obj);

          if (hasSiv)
            obj.push_back(ENCODE_D3D10_SB_NAME(siv));
        });
      }

      void writeDcls(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) {
        auto& obj = bytecode.getBytecodeVector();

        // Global Flags
        if (config::getBool(config::RefactoringAllowed))
        {
          DXBCOperation{ D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS, false, 1, 0, UINT32_MAX, D3D10_SB_GLOBAL_FLAG_REFACTORING_ALLOWED }.push(obj);
        }

        // Temps
        uint32_t tempCount = shdrCode.getRegisterMap().getTotalTempCount();
        if (tempCount > 0)
        {
          DXBCOperation{ D3D10_SB_OPCODE_DCL_TEMPS, false, 2 }.push(obj);
          obj.push_back(tempCount); // Followed by DWORD count of temps. Not an operand!
        }
        
        // Samplers
        {
          for (uint32_t i = 0; i < 16; i++) {
            if (shdrCode.isSamplerUsed(i)) {
              SamplerDesc* sampler = shdrCode.getSampler(i);

              DXBCOperand samplerOp{ D3D10_SB_OPERAND_TYPE_SAMPLER, 1 };
              samplerOp.setComponents(0);
              samplerOp.setSwizzleOrWritemask(noSwizzle);
              samplerOp.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
              samplerOp.setData(&i, 1);

              DXBCOperation{ D3D10_SB_OPCODE_DCL_SAMPLER, false }
                .appendOperand(samplerOp)
                .push(obj);

              DXBCOperand textureOp{ D3D10_SB_OPERAND_TYPE_RESOURCE, 1 };
              textureOp.setData(&i, 1);
              textureOp.setSwizzleOrWritemask(noSwizzle);
              textureOp.setComponents(0);
              textureOp.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);

              DXBCOperation{ D3D10_SB_OPCODE_DCL_RESOURCE, false, UINT32_MAX, 1 }
                .setExtra(ENCODE_D3D10_SB_RESOURCE_DIMENSION(sampler->dimension))
                .appendOperand(textureOp)
                .push(obj);

              // Return type token.
              uint32_t returnTypeToken = 0;
              returnTypeToken |= ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(D3D10_SB_RETURN_TYPE_FLOAT, 0);
              returnTypeToken |= ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(D3D10_SB_RETURN_TYPE_FLOAT, 1);
              returnTypeToken |= ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(D3D10_SB_RETURN_TYPE_FLOAT, 2);
              returnTypeToken |= ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(D3D10_SB_RETURN_TYPE_FLOAT, 3);
              obj.push_back(returnTypeToken);
            }
          }
        }

        // Input
        writeIODcls<true>(bytecode, shdrCode);

        // Output
        writeIODcls<false>(bytecode, shdrCode);

        // Constant Buffer
        {
          const uint32_t constantBuffer = 0;
          uint32_t cbufferCount = 0;

          if (shdrCode.isIndirectMarked())
            cbufferCount = 256 + 16;
          else
            cbufferCount = shdrCode.getRegisterMap().getDXBCTypeCount(D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER);

          if (cbufferCount != 0) {
            uint32_t data[2] = { constantBuffer , cbufferCount };
            DXBCOperation{ D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER, false }
              .appendOperand(DXBCOperand{ D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER, 2 }.setSwizzleOrWritemask(noSwizzle).setData(data, 2))
              .setExtra(ENCODE_D3D10_SB_D3D10_SB_CONSTANT_BUFFER_ACCESS_PATTERN(shdrCode.isIndirectMarked() ? D3D10_SB_CONSTANT_BUFFER_DYNAMIC_INDEXED : D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED))
              .push(obj);
          }
        }

      }

      void pushInternal(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) override {
        auto& obj = bytecode.getBytecodeVector();

        PlaceholderPtr<uint32_t> headerChunkSize{ "[SHEX] Chunk Header - Chunk Data Size", &((ChunkHeader*)nextPtr(obj))->size };
        ChunkHeader{ fourcc("SHEX") }.push(obj); // [PUSH] Chunk Header
        
        obj.push_back(ENCODE_D3D10_SB_TOKENIZED_PROGRAM_VERSION_TOKEN(shdrCode.getShaderType() == ShaderType::Vertex ? D3D10_SB_VERTEX_SHADER : D3D10_SB_PIXEL_SHADER, 5, 0)); // [PUSH] DXBC Version Token - VerTok

        PlaceholderPtr<uint32_t> dwordCount{ "[SHEX] Dword Count", nextPtr(obj) };
        obj.push_back(0); // [PUSH] Dword Count

        writeDcls(bytecode, shdrCode);

        for (uint32_t token : shdrCode.getCode())
          obj.push_back(token);

        headerChunkSize = this->getChunkSize(bytecode);
        dwordCount = this->getChunkSize(bytecode) / sizeof(uint32_t);
      }
    };

    class STATChunk : public BaseChunk<chunks::STAT> {
      void pushInternal(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) override {
        auto& obj = bytecode.getBytecodeVector();

        PlaceholderPtr<uint32_t> headerChunkSize{ "[STAT] Chunk Header - Chunk Data Size", &((ChunkHeader*)nextPtr(obj))->size };
        ChunkHeader{ fourcc("STAT") }.push(obj); // [PUSH] Chunk Header
        STATData statData;
        pushObject(obj, statData); // [PUSH] Dummy Stat Data

        headerChunkSize = this->getChunkSize(bytecode);
      }
    };

    template <uint32_t ChunkType>
    class IOSGNChunk : public BaseChunk<ChunkType> {

      // Pack properly for DXBC formatting...
      struct IOSGNElement {
        uint32_t nameOffset;
        uint32_t semanticIndex;
        uint32_t systemValueType = D3D_NAME_UNDEFINED;
        uint32_t componentType = D3D_SVT_FLOAT;
        uint32_t registerIndex;
        uint32_t mask = 0;
      };

      void pushInternal(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode) override {
        auto& obj = bytecode.getBytecodeVector();

        PlaceholderPtr<uint32_t> headerChunkSize{ "[IOSGN] Chunk Header - Chunk Data Size", &((ChunkHeader*)nextPtr(obj))->size };
        ChunkHeader{ fourcc(ChunkType == chunks::ISGN ? "ISGN" : "OSGN") }.push(obj); // [PUSH] Chunk Header

        PlaceholderPtr<uint32_t> elementCount{ "[ISOGN] Element Count", nextPtr(obj) };
        obj.push_back(0); // [PUSH] Element Count
        obj.push_back(8); // [PUSH] Magic 8-Ball Constant ~ Reply hazy, try again.

        IOSGNElement* elementStart = (IOSGNElement*)nextPtr(obj);

        if (shdrCode.isTransient(isInput(ChunkType))) {
          for (auto& transMapping : RegisterMap::getTransientMappings()) {
            IOSGNElement element;

            element.nameOffset = 0; // <-- Must be set later!
            element.registerIndex = transMapping.dxbcRegNum;
            element.semanticIndex = transMapping.dxbcSemanticIndex;

            uint32_t baseMask = 0;
            forEachValidElement<isInput(ChunkType)>(bytecode, shdrCode, [&](const RegisterMapping& mapping, uint32_t i) {
              if (mapping.dclInfo.usage == transMapping.d3d9Usage && mapping.dclInfo.usageIndex == transMapping.d3d9UsageIndex)
                baseMask |= isInput(ChunkType) ? mapping.readMask : mapping.writeMask;
            });
            
            baseMask = DECODE_D3D10_SB_OPERAND_4_COMPONENT_MASK(baseMask);
            element.mask = 0xFFu;
            uint32_t rwMask = baseMask >> D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT;

            if (!isInput(ChunkType))
              rwMask = rwMask ^ 0xFFu;

            rwMask = rwMask << 8;
            element.mask |= rwMask;

            pushObject(obj, element);
          }

          uint32_t count = 0;
          for (auto& transMapping : RegisterMap::getTransientMappings()) {
            elementStart[count].nameOffset = this->getChunkSize(bytecode);
            pushAlignedString(obj, transMapping.dxbcSemanticName);
            count++;
          }

          elementCount = count;
        }
        else {
          forEachValidElement<isInput(ChunkType)>(bytecode, shdrCode, [&](const RegisterMapping& mapping, uint32_t i) {
            IOSGNElement element;

            element.nameOffset = 0; // <-- Must be set later!
            element.registerIndex = mapping.dxbcOperand.getRegNumber();
            element.semanticIndex = mapping.dclInfo.usageIndex;

            uint32_t baseMask = isInput(ChunkType) ? mapping.readMask : mapping.writeMask;
            baseMask = DECODE_D3D10_SB_OPERAND_4_COMPONENT_MASK(baseMask);
            element.mask = baseMask >> D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT;
            uint32_t rwMask = element.mask;

            if (!isInput(ChunkType))
              rwMask = rwMask ^ 0xFFu;

            rwMask = rwMask << 8;
            element.mask |= rwMask;

            element.systemValueType = convert::sysValue(ChunkType == chunks::ISGN, mapping.dclInfo.target, mapping.dclInfo.usage);
            pushObject(obj, element);
          });

          uint32_t count = 0;
          forEachValidElement<isInput(ChunkType)>(bytecode, shdrCode, [&](const RegisterMapping& mapping, uint32_t i) {
            elementStart[i].nameOffset = this->getChunkSize(bytecode);
            pushAlignedString(obj, convert::declUsage(ChunkType == chunks::ISGN, mapping.dclInfo.target, mapping.dclInfo.usage));

            count++;
          });

          elementCount = count;
        }

        headerChunkSize = this->getChunkSize(bytecode);
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