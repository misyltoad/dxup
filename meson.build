project('dxup', ['c', 'cpp'])

cpu_family = target_machine.cpu_family()

dxup_compiler = meson.get_compiler('cpp')
if dxup_compiler.get_id() == 'msvc'
  add_global_arguments('-DNOMINMAX', language : 'cpp')
  dxup_cpp_std='c++latest'
else
  dxup_cpp_std='c++1z'
endif

if dxup_compiler.get_id() == 'msvc'
	add_global_arguments('/std:' + dxup_cpp_std, language : 'cpp')
endif

lib_d3d11   = dxup_compiler.find_library('d3d11')

subdir('src')

if dxup_compiler.get_id() != 'msvc'
  subdir('wine_utils')
endif
