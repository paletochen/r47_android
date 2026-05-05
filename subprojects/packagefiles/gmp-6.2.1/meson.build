project('gmp', 'c',
  version       : '6.2.1',
  meson_version : '>=0.60.0',
  license       : ['LGPL-3.0-only', 'GPL-2.0-only'],
)

env = environment()
env.append('CFLAGS',
  '-Os',
  separator : ' ')

env.set('CC', 'arm-none-eabi-gcc')
mod = import('unstable-external_project')

if build_machine.system() == 'windows'
  prefix = '/mingw64'
  p = mod.add_project(
    'configure',
    configure_options : [
      '--prefix=@PREFIX@',
      '--prefix='     + prefix,
      '--libdir='     + prefix + '/@LIBDIR@',
      '--includedir=' + prefix + '/@INCLUDEDIR@',
      '--bindir='     + prefix + '/@BINDIR@',
      '--disable-cxx'],
    cross_configure_options : [
      '--host=arm-none-eabi',
      '--disable-assembly',
      '--disable-shared',
      '--disable-fft'],
    env : env
  )
else
  prefix = ''
  p = mod.add_project(
    'configure',
    configure_options : [
      '--prefix=@PREFIX@',
      '--prefix='     + prefix,
      '--libdir='     + prefix + '/@LIBDIR@',
      '--includedir=' + prefix + '/@INCLUDEDIR@',
      '--disable-cxx'],
    cross_configure_options : [
      '--host=arm-none-eabi',
      '--disable-assembly',
      '--disable-shared',
      '--disable-fft'],
    env : env
  )
endif


arm_gmp_dep = p.dependency('gmp')
meson.override_dependency('arm-gmp-6.2.1', arm_gmp_dep)

