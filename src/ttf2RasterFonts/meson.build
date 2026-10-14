font_files = files(
  '../../res/fonts/C47__NumericFont.ttf',
  '../../res/fonts/C47__StandardFont.ttf',
  '../../res/fonts/C47__TinyFont.ttf'
)

ttf2RasterFonts = executable('ttf2RasterFonts',
  build_by_default : false,
  sources          : files('ttf2RasterFonts.c'),
  c_args              : '-DGENERATE_FONTS',
  dependencies     : ft_dep,
  native           : true
)

rasterFontsData_c = custom_target('rasterFontsData.c',
  build_by_default : false,
  input            : font_files,
  output           : 'rasterFontsData.c',
  command          : [ttf2RasterFonts, '@CURRENT_SOURCE_DIR@/../../res/fonts', '@OUTPUT@']
)

testTtf2RasterFonts = executable('testTtf2RasterFonts',
  build_by_default    : false,
  sources             : files('testTtf2RasterFonts.c') + [rasterFontsData_c],
  c_args              : '-DGENERATE_FONTS',
  include_directories : [decNumber_inc, c47_inc],
  dependencies        : [gtk_dep],
  native              : true
)
