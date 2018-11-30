#include "dx9asm_translator.h"
#include "dx9asm_operations.h"
#include "../util/misc_helpers.h"
#include "../util/log.h"
#include "../util/config.h"
#include "dx9asm_operation_helpers.h"
#include "dxbc_helpers.h"
#include <vector>
#include <functional>

namespace dxapex {

  namespace dx9asm {

    bool ShaderCodeTranslator::handleComment(DX9Operation& operation) {
      uint32_t commentTokenCount = operation.getCommentCount();

      uint32_t fourcc = nextToken();
      if (fourcc == CTAB_CONSTANT) {
        m_ctab = (CTHeader*)(m_head);
        uint32_t tableSize = (commentTokenCount - 1) * sizeof(uint32_t);

        if (tableSize < sizeof(CTHeader) || m_ctab->size != sizeof(CTHeader)) {
          log::fail("CTAB invalid!");
          return false; // fatal
        }
      }

      skipTokens(commentTokenCount - 1);
      return true;
    }

    bool ShaderCodeTranslator::handleDef(DX9Operation& operation) {
      const DX9Operand* dst = operation.getOperandByType(optype::Dst);
      const DX9Operand* vec4 = operation.getOperandByType(optype::Vec4);

      RegisterMapping mapping;
      mapping.dx9Id = dst->getRegNumber();
      mapping.dx9Type = dst->getRegType();
      mapping.dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_IMMEDIATE32);
      mapping.dxbcOperand.setExtension(false);
      mapping.dxbcOperand.setDimension(D3D10_SB_OPERAND_INDEX_0D);
      uint32_t data[4];
      vec4->getValues(data);
      mapping.dxbcOperand.setData(data, 4);
      for (uint32_t i = 0; i < 4; i++)
        mapping.dxbcOperand.setRepresentation(i, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);

      mapping.dxbcOperand.setSwizzleOrWritemask(noSwizzle);

      addRegisterMapping(false, mapping);

      return true;
    }

    bool ShaderCodeTranslator::handleUniqueOperation(DX9Operation& operation) {
      UniqueFunction function = operation.getUniqueFunction();
      if (function == nullptr) {
        log::fail("Unimplemented operation %s encountered.", operation.getName());
        return true; // nonfatal
      }

      return std::invoke(function, this, operation);
    }

    bool ShaderCodeTranslator::handleStandardOperation(DX9Operation& operation) {
      if (!config::getBool(config::EmitNop) && operation.getImplicitInfo().dxbcOpcode == D3D10_SB_OPCODE_NOP)
        return true;

      for (uint32_t col = 0; col < operation.getMatrixColumns(); col++) {
        DXBCOperation dxbcOperation{operation};

        for (uint32_t i = 0; i < operation.operandCount(); i++) {
          const DX9Operand* operand = operation.getOperandByIndex(i);

          uint32_t regOffset = operand->getType() == optype::Src1 ? col : 0;
          DXBCOperand dbxcOperand{ *this, operation, *operand, regOffset };
          dxbcOperation.appendOperand(dbxcOperand);
        }

        dxbcOperation.push(*this);
      }

      return true;
    }

    bool ShaderCodeTranslator::handleOperation(uint32_t token) {
      DX9Operation operation{ *this, token };
      if (!operation.isValid()) {
        log::fail("Unknown operation encountered!");
        return false;
      }

      if (config::getBool(config::ShaderSpew))
        log::msg("Translating operation %s.", operation.getName());

      if (operation.isUnique())
        return handleUniqueOperation(operation);
      else
        return handleStandardOperation(operation);

      return true;
    }

    bool ShaderCodeTranslator::translate() {
      nextToken(); // Skip header.

      uint32_t token = nextToken();
      while (token != D3DPS_END()) {
        if (!handleOperation(token))
          return false;
        token = nextToken();
      }

      return true;
    }

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

    struct DXBCHeader {
      uint32_t dxbc = MAKEFOURCC('D', 'X', 'B', 'C');
      uint32_t checksum[4] = { 0 };
      uint32_t unknown = 1;
      uint32_t size = 0; // Set later.
      uint32_t chunkCount = chunks::Count;
      uint32_t chunkOffsets[chunks::Count] = { 0 }; // Set later.
    };

    struct Chunk {
      Chunk(uint32_t name)
        : name(name) {}

      uint32_t name = MAKEFOURCC('I', 'V', 'L', 'D');
      uint32_t size = 0; // Set Later
    };

    struct RDEF : public Chunk {
      RDEF(ShaderCodeTranslator& shdrCode) : Chunk{ MAKEFOURCC('R', 'D', 'E', 'F') }  {
        size = sizeof(RDEF);
      }

      void push(std::vector<uint32_t>& obj) {
        obj.reserve(obj.size() + size);

        obj.push_back(name);
        obj.push_back(size);
      }
    };

    struct ISGN : public Chunk {
      ISGN(ShaderCodeTranslator& shdrCode) : Chunk{ MAKEFOURCC('I', 'S', 'G', 'N') } {
        size = sizeof(ISGN);
      }

      void push(std::vector<uint32_t>& obj) {
        obj.reserve(obj.size() + size);

        obj.push_back(name);
        obj.push_back(size);
      }
    };

    struct OSGN : public Chunk {
      OSGN(ShaderCodeTranslator& shdrCode) : Chunk{ MAKEFOURCC('O', 'S', 'G', 'N') } {
        size = sizeof(OSGN);
      }

      void push(std::vector<uint32_t>& obj) {
        obj.reserve(obj.size() + size);

        obj.push_back(name);
        obj.push_back(size);
      }
    };

    struct SHDR : public Chunk {
      SHDR(ShaderCodeTranslator& shdrCode) : Chunk{ MAKEFOURCC('S', 'H', 'D', 'R') } {

        versionAndType = ENCODE_D3D10_SB_TOKENIZED_PROGRAM_VERSION_TOKEN(D3D10_SB_VERTEX_SHADER, 4, 0);
        code = &shdrCode.getCode();

        size = sizeof(SHDR) - sizeof(code) + (code->size() * sizeof(uint32_t));
        dwordCount = sizeof(versionAndType) + sizeof(dwordCount) + (code->size() * sizeof(uint32_t));
      }

      void push(std::vector<uint32_t>& obj) {
        obj.reserve(obj.size() + size);

        obj.push_back(name);
        obj.push_back(size);
        obj.push_back(versionAndType);
        obj.push_back(dwordCount);

        for (size_t i = 0; i < code->size(); i++)
          obj.push_back((*code)[i]);
      }

      uint32_t versionAndType;
      uint32_t dwordCount;
      const std::vector<uint32_t>* code;
    };

    struct STAT : public Chunk {
      STAT(ShaderCodeTranslator& shdrCode) : Chunk{ MAKEFOURCC('S', 'T', 'A', 'T') } {
        size = sizeof(STAT);
      }

      void push(std::vector<uint32_t>& obj) {
        obj.reserve(obj.size() + size);

        obj.push_back(name);
        obj.push_back(size);
      }
    };
    
    class ShaderBytecode {
    public:
      ShaderBytecode(ShaderCodeTranslator& shdrCode) {
        pushObjectAsBytes(DXBCHeader());

        getHeader()->chunkOffsets[chunks::RDEF] = getByteSize();
        RDEF(shdrCode).push(m_bytecode);

        getHeader()->chunkOffsets[chunks::ISGN] = getByteSize();
        ISGN(shdrCode).push(m_bytecode);

        getHeader()->chunkOffsets[chunks::OSGN] = getByteSize();
        OSGN(shdrCode).push(m_bytecode);

        getHeader()->chunkOffsets[chunks::SHDR] = getByteSize();
        SHDR(shdrCode).push(m_bytecode);

        getHeader()->chunkOffsets[chunks::STAT] = getByteSize();
        STAT(shdrCode).push(m_bytecode);

        getHeader()->size = getByteSize();
        // Do Checksum Here
      }

      DXBCHeader* getHeader() {
        return (DXBCHeader*) &m_bytecode[0];
      }

      uint32_t getByteSize() {
        return m_bytecode.size() * sizeof(uint32_t);
      }

      template <typename T>
      void pushObjectAsBytes(T& thing) {
        uint32_t* ptrThing = (uint32_t*) &thing;
        for (uint32_t i = 0; i < sizeof(T) / sizeof(uint32_t); i++)
          m_bytecode.push_back( *(ptrThing + i) );
      }
    private:
      std::vector<uint32_t> m_bytecode;
    };
  
    void toDXBC(const uint32_t* dx9asm, ITranslatedShaderDXBC** dxbc) {
      InitReturnPtr(dxbc);

      ShaderCodeTranslator translator{dx9asm};
      if (!translator.translate()) {
        log::fail("Failed to translate shader fatally!");
        return;
      }

      
    }

  }

}