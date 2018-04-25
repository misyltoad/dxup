d3d10_src = [
  'd3d10_main.cpp',
  '../d3d10_shared/d3d10_shared_main.cpp',
]

d3d10_dll = shared_library('d3d10', d3d10_src,
  name_prefix         : '',
  dependencies        : [ d3d10_1_dep ],
  install             : true,
  vs_module_defs      : 'd3d10.def',
  override_options    : ['cpp_std='+dxup_cpp_std])

d3d10_dep = declare_dependency(
  link_with           : [ d3d10_dll, d3d10_1_dll ],
  include_directories : [ include_directories('.') ])
