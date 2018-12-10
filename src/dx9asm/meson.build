dx9asm_src = files([
  'dx9asm_operations.cpp',
  'dx9asm_translator.cpp',
  'dx9asm_util.cpp',
  'dx9asm_modifiers.cpp',
  'dx9asm_operand.cpp',
  'dx9asm_unique_operations.cpp',
  'dx9asm_standard_operations.cpp',
  'dx9asm_register_map.cpp',
  'dxbc_bytecode.cpp',
  'dxbc_helpers.cpp',
  'dxbc_chunks.cpp',
  '../extern/gpuopen/DXBCChecksum.cpp'
])

dx9asm_lib = static_library('dx9asm', dx9asm_src,
  override_options    : ['cpp_std='+dxup_cpp_std])

dx9asm_dep = declare_dependency(
link_with           : [ dx9asm_lib ])