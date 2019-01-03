util_src = files([
  'private_data_d3d.cpp',
  'log.cpp',
  'config.cpp',
  'shared_conversions.cpp',
  'thread.cpp',
  'd3dcompiler_helpers.cpp',
  '../extern/smhasher/MurmurHash3.cpp'
])

util_lib = static_library('util', util_src,
  override_options    : ['cpp_std='+dxup_cpp_std])

util_dep = declare_dependency(
link_with           : [ util_lib ])