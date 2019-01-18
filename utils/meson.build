conf = configuration_data()
conf.set('bindir', get_option('bindir'))
conf.set('libdir', get_option('libdir'))
conf.set('arch', target_machine.cpu_family())
conf.set('winelib', dxup_winelib)
configure_file(
  configuration : conf,
  input         : 'setup_dxup_d3d9.sh.in',
  output        : 'setup_dxup_d3d9.sh',
  install_dir   : get_option('bindir')
)