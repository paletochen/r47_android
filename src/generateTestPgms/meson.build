generateTestPgms = executable('generateTestPgms',
  build_by_default    : false,
  sources             : files('generateTestPgms.c') + generateTestPgms_src + decNumber_src,
  c_args              : '-DGENERATE_TESTPGMS',
  include_directories : [decNumber_inc, c47_inc],
  dependencies        : [gtk_dep, gmp_dep],
  native              : true
)

testPgms_bin = custom_target('testPgms.bin',
  build_by_default : false,
  output           : 'testPgms.bin',
  command          : [generateTestPgms, '@OUTPUT@']
)

alias_target('testPgms', testPgms_bin)