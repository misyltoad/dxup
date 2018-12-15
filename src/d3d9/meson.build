d3d9_main_src = [
  'd3d9_device.cpp',
  'd3d9_interface.cpp',
  'd3d9_main.cpp',
  'd3d9_surface.cpp',
  'd3d9_swapchain.cpp',
  'd3d9_texture.cpp',
  'd3d9_util.cpp',
  'd3d9_format.cpp',
  'd3d9_constant_buffer.cpp'
]

d3d9_deps = [ lib_d3d11, lib_dxgi, util_dep, dx9asm_dep ]

d3d9_dll = shared_library('d3d9'+dll_ext, d3d9_main_src,
  name_prefix         : '',
  dependencies        : [ d3d9_deps ],
  install             : true,
  objects             : not dxup_msvc ? 'd3d9'+def_spec_ext : [],
  vs_module_defs      : 'd3d9'+def_spec_ext,
  override_options    : ['cpp_std='+dxup_cpp_std]
)

d3d9_dep = declare_dependency(
  link_with           : [ d3d9_dll ])