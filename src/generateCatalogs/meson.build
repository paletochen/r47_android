generateCatalogs = executable('generateCatalogs',
  build_by_default    : false,
  sources             : files('generateCatalogs.c') + generateCatalogs_src + [rasterFontsData_c],
  c_args              : '-DGENERATE_CATALOGS',
  include_directories : [decNumber_inc, c47_inc],
  dependencies        : [gtk_dep, gmp_dep],
  native              : true
)

softmenuCatalogs_h = custom_target('softmenuCatalogs.h',
  build_by_default : false,
  output           : 'softmenuCatalogs.h',
  command          : [generateCatalogs, '@OUTPUT@']
)
