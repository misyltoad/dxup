conf = configuration_data()
conf.set('dlldir', get_option('prefix')+'/'+get_option('bindir'))
conf.set('arch', target_machine.cpu_family())
configure_file(
  configuration : conf,
  input         : 'setup_dx10to11.sh.in',
  output        : 'setup_dx10to11.sh',
  install_dir   : get_option('bindir')
)
