util_src = files([
  'private_data_d3d.cpp',
  'log.cpp',
  'config.cpp',
  'shared_conversions.cpp',
  'd3dcompiler_helpers.cpp'
])

util_lib = static_library('util', util_src, dxup_version,
  override_options    : ['cpp_std='+dxup_cpp_std])

util_dep = declare_dependency(
link_with           : [ util_lib ])