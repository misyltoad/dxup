d3d10_1_src = [
  '../d3d10_shared/d3d10_shared_main.cpp',
  'd3d10_1_blend.cpp',
  'd3d10_1_buffer.cpp',
  'd3d10_1_device.cpp',
  'd3d10_1_input_layout.cpp',
  'd3d10_1_main.cpp',
  'd3d10_1_query.cpp',
  'd3d10_1_texture.cpp',
  'd3d10_1_view_dsv.cpp',
  'd3d10_1_view_rtv.cpp',
  'd3d10_1_view_srv.cpp',
  'logger.cpp'
]

d3d10_1_dll = shared_library('d3d10_1', d3d10_1_src,
  name_prefix         : '',
  install             : true,
  dependencies        : [ lib_d3d11 ],
  vs_module_defs      : 'd3d10_1.def',
  cpp_args			  : '-DDXUP_EXPORTS',
  override_options    : ['cpp_std='+dxup_cpp_std])

d3d10_1_dep = declare_dependency(
  link_with           : [ d3d10_1_dll ],
  include_directories : [ include_directories('.') ])
