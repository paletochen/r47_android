project('gmp', 'c',
  version       : '6.2.1',
  meson_version : '>=0.60.0',
  license       : ['LGPL-3.0-only', 'GPL-2.0-only'],
)

env = environment()
env.append('CFLAGS',
  '-Os',
  '-std=gnu17',
  '-Wno-overflow',  # the MP_SIZE_T_MAX thresholds of mpn/generic/gmp-mparam.h overflow in constant expressions whose values go unused or only pick a split point
  separator : ' ')

mod = import('unstable-external_project')

if build_machine.system() == 'windows'
  env.set('CC', 'arm-none-eabi-gcc')
  env.set('ac_cv_sizeof_mp_limb_t', '4')
  prefix = '/ucrt64'
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
      '--disable-debug',
      '--disable-dependency-tracking',
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
      '--disable-debug',
      '--disable-dependency-tracking',
      '--host=arm-none-eabi',
      '--disable-assembly',
      '--disable-shared',
      '--disable-fft'],
    env : env
  )
endif

arm_gmp_dep = p.dependency('gmp')
meson.override_dependency('arm-gmp-6.2.1', arm_gmp_dep)
