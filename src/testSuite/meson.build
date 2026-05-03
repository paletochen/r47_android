testSuite_src = files(
  'testSuite.c',
  'hal/audio.c',
  'hal/gui.c',
  'hal/lcd.c',
  'hal/io.c')

testSuite = executable('testSuite',
  build_by_default    : false,
  sources             : decNumber_src + c47_src + testSuite_src + [rasterFontsData_c, softmenuCatalogs_h, constantPointers_ch],
  c_args              : '-DTESTSUITE_BUILD',
  include_directories : [decNumber_inc, c47_inc],
  dependencies        : [gtk_dep, gmp_dep, m_dep],
  native              : true
)

test(
  'testSuite',
  testSuite,
  args    : [files('tests/testSuiteList.txt')],
  workdir : meson.project_source_root(),
  timeout : 800
)
