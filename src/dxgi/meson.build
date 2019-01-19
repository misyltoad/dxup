dxgi_src = [
  'dxgi_factory.cpp',
  'dxgi_main.cpp',
  'dxgi_swapchain.cpp',
  '../dxup_guids.cpp'
]

dxgi_dll = shared_library('dxgi'+dll_ext, dxgi_src,
  name_prefix         : '',
  dependencies        : [ d3d10_1_dep ],
  install             : false,
  vs_module_defs      : 'dxgi'+def_spec_ext,
  objects             : not dxup_msvc ? 'dxgi'+def_spec_ext : [],
  cpp_args	      : '-DDXUP_DXGI_EXPORTS',
  override_options    : ['cpp_std='+dxup_cpp_std])

dxgi_dep = declare_dependency(
  link_with           : [ dxgi_dll ],
  include_directories : [ include_directories('.') ])
