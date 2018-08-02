project('dxup', ['c', 'cpp'])

cpu_family = target_machine.cpu_family()

add_project_arguments('-DNOMINMAX', language : 'cpp')

dxup_compiler = meson.get_compiler('cpp')
if dxup_compiler.get_id() == 'msvc'
  dxup_cpp_std='c++latest'
  dxup_msvc=true
else
  dxup_cpp_std='c++1z'
  dxup_msvc=false
endif

if dxup_compiler.get_id() == 'msvc'
	add_global_arguments('/std:' + dxup_cpp_std, language : 'cpp')
endif

if meson.get_cross_property('winelib', false)
  lib_d3d11   = declare_dependency(link_args: [ '-ld3d11' ])
  lib_d3dcompiler_47 = declare_dependency(link_args: [ '-ld3dcompiler' ])
  exe_ext = '.exe.so'
  dll_ext = '.dll'
  def_spec_ext = '.spec'
else
  lib_d3d11  = dxup_compiler.find_library('d3d11')
  lib_d3dcompiler_47  = dxup_compiler.find_library('d3dcompiler')
  exe_ext = ''
  dll_ext = ''
  def_spec_ext = '.def'
endif

subdir('src')

if dxup_compiler.get_id() != 'msvc'
  subdir('wine_utils')
endif
