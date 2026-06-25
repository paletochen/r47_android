decNumber_src = files(
  'decNumberICU/decContext.c',
  'decNumberICU/decDouble.c',
  'decNumberICU/decimal128.c',
  'decNumberICU/decimal64.c',
  'decNumberICU/decNumber.c',
  'decNumberICU/decQuad.c'
)
decNumber_inc = include_directories('decNumberICU')

if get_option('DMCPVERSION') == 'dmcp'
  dmcp_src = files(
    'DMCP_SDK/dmcp/sys/pgm_syscalls.c',
    'DMCP_SDK/dmcp/startup_pgm.s'
  )
  dmcp_inc = include_directories('DMCP_SDK/dmcp')
endif

if get_option('DMCPVERSION') == 'dmcp5'
  dmcp_src = files(
    'DMCP5_SDK/dmcp/sys/pgm_syscalls.c',
    'DMCP5_SDK/dmcp/startup_pgm.s'
  )
  dmcp_inc = include_directories('DMCP5_SDK/dmcp')
endif

forcecrc32 = executable('forcecrc32',
  build_by_default : false,
  sources          : files('forcecrc32.c'),
  native           : true
)
