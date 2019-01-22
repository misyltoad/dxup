project('dxup', ['c', 'cpp'], version : 'v0.00', meson_version : '>= 0.43')

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


if (cpu_family == 'x86_64')
  dxup_library_path = meson.source_root() + '/lib'
else
  dxup_library_path = meson.source_root() + '/lib32'
endif

code = '''#ifndef __WINE__
#error 1
#endif'''

dxup_winelib = dxup_compiler.compiles(code, name: 'winelib check')
dxup_extradep = [ ]

if dxup_winelib
  lib_d3d11   = declare_dependency(link_args: [ '-ld3d11' ])
  lib_dxgi    = declare_dependency(link_args: [ '-ldxgi' ])
  lib_shlwapi    = declare_dependency(link_args: [ '-lshlwapi' ])
  lib_d3dcompiler_43 = declare_dependency(link_args: [ '-L'+dxup_library_path, '-ld3dcompiler_43' ])
  lib_d3dcompiler_47 = declare_dependency(link_args: [ '-ld3dcompiler' ])
  dxup_extradep += [ declare_dependency(link_args: [ '-ldl' ]) ]
  exe_ext = '.exe.so'
  dll_ext = '.dll'
  def_spec_ext = '.spec'
else
  lib_d3d11   = dxup_compiler.find_library('d3d11')
  lib_dxgi    = dxup_compiler.find_library('dxgi')
  lib_shlwapi    = dxup_compiler.find_library('shlwapi')
  lib_d3dcompiler_43 = dxup_compiler.find_library('d3dcompiler_43', dirs : dxup_library_path)
  if dxup_compiler.get_id() == 'msvc'
    lib_d3dcompiler_47 = dxup_compiler.find_library('d3dcompiler')
  else
    lib_d3dcompiler_47 = dxup_compiler.find_library('d3dcompiler_47')
  endif

  exe_ext = ''
  dll_ext = ''
  def_spec_ext = '.def'
endif

dxup_version = vcs_tag(
  command: ['git', 'describe', '--always', '--dirty=+'],
  input:  'version.h.in',
  output: 'version.h')

subdir('src')

if dxup_compiler.get_id() != 'msvc'
  subdir('utils')
endif
