dmcp_cargs = [
  '-D__weak="__attribute__((weak))"',
  '-D__packed="__attribute__((__packed__))"',
  '-DNEW_HW',
  '-Wno-unused-parameter',
  '-fno-exceptions',                 # added in symmetry with DM42
  '-fno-unwind-tables',              # added in symmetry with DM42
  '-fno-asynchronous-unwind-tables', # added in symmetry with DM42
]

dmcp_linkargs = [
  '--specs=nano.specs',              # can remove full math later to save space
  '--specs=nosys.specs',
  '-u', '_printf_float',             # added in symmetry with DM42 - Enable printf
#  '-u', '_scanf_float',              # added in symmetry with DM42 - Enable float scanf
  '-lc',                             # added in symmetry with DM42 - Link C standard library
  '-lm',                             # added in symmetry with DM42 - Link math library (needed for float ops)
  '-lnosys',                         # added in symmetry with DM42 - Stub system calls for bare-metal
  '-Wl,--gc-sections',
  '-Wl,--wrap=_malloc_r',
  '-Wl,-Map=' + meson.current_build_dir() + '/C47.map,--cref',
  '-T' + meson.current_source_dir() + '/stm32_program.ld'
#  '-Wl,--strip-all',
]

gmp_build_dir = meson.project_build_root() / 'subprojects' / 'gmp-6.2.1' / 'build'
gmp_lib_dir = meson.project_build_root() / 'subprojects' / 'gmp-6.2.1' / 'build' / '.libs'

arm_gmp_dep = declare_dependency(
  dependencies : [dependency('arm-gmp-6.2.1')],
  compile_args : ['-I' + gmp_build_dir],
  link_args    : ['-L' + gmp_lib_dir],
)

#
# C47 Build : standard build for the DM32 board with the C47 bezel and DM42 keys or the DM42n
#

c47_dmcp_src = files(
  'hal/audio.c',
  'hal/io.c'
)

c47_lib = static_library('libc47',
  build_by_default    : false,
  sources             : decNumber_src + c47_src + c47_dmcp_src + [rasterFontsData_c, softmenuCatalogs_h, constantPointers_ch, vcs_h, version_h],
  c_args              : dmcp_cargs,
  include_directories : [decNumber_inc, c47_inc, dmcp_inc],
  dependencies        : arm_gmp_dep,
  pic                 : false,
  override_options    : ['debug=false', 'optimization=s']
)

# First pass building the ELF to get the size of the QSPI
c47_pre_elf = executable('C47_pre.elf',
  build_by_default    : false,
  sources             : dmcp_src + [vcs_h, version_h],
  c_args              : dmcp_cargs,
  link_args           : dmcp_linkargs,
  dependencies        : arm_gmp_dep,
  link_with           : c47_lib,
  include_directories : [c47_inc, dmcp_inc],
  pie                 : false,
  override_options    : ['debug=false', 'optimization=s']
)

c47_pre_qspi_incorrect_crc = custom_target(
  'C47_pre_qspi_incorrect_crc.bin',
  build_by_default : false,
  input            : [c47_pre_elf],
  output           : 'C47_pre_qspi_incorrect_crc.bin',
  command          : [arm_objcopy, '--only-section', '.qspi', '-O', 'binary', '@INPUT@', '@OUTPUT@']
)

c47_pre_qspi = custom_target(
  'C47_pre_qspi.bin',
  build_by_default : false,
  input            : [c47_pre_qspi_incorrect_crc],
  output           : 'C47_pre_qspi.bin',
  command          : [bash, modify_crc, forcecrc32, '@INPUT@', '@OUTPUT@']
)

generated_qspi_crc_h = custom_target(
  'generated_qspi_crc.h',
  build_by_default : false,
  input            : [c47_pre_qspi],
  output           : 'generated_qspi_crc.h',
  command          : [bash, gen_qspi_crc, '@INPUT@', '@OUTPUT@']
)

# Second pass building the ELF including the correct qspi_crc.h
c47_elf = executable('C47.elf',
  build_by_default    : false,
  sources             : dmcp_src + [vcs_h, version_h, generated_qspi_crc_h],
  c_args              : dmcp_cargs + ['-DUSE_GEN_QSPI_CRC'],
  link_args           : dmcp_linkargs,
  dependencies        : arm_gmp_dep,
  link_with           : c47_lib,
  include_directories : [c47_inc, dmcp_inc],
  pie                 : false,
  override_options    : ['debug=false', 'optimization=s']
)

c47_flash = custom_target(
  'C47_flash.bin',
  build_by_default : false,
  input            : [c47_elf],
  output           : 'C47_flash.bin',
  command          : [arm_objcopy, '--remove-section', '.qspi', '-O', 'binary', '@INPUT@', '@OUTPUT@']
)

c47_pg5 = custom_target(
  'C47.pg5',
  build_by_default : false,
  input            : [c47_flash],
  output           : 'C47.pg5',
  command          : [bash, add_pgm_chsum, '@INPUT@', '@OUTPUT@']
)

c47_qspi_incorrect_crc = custom_target(
  'C47_qspi_incorrect_crc.bin',
  build_by_default : false,
  input            : [c47_elf],
  output           : 'C47_qspi_incorrect_crc.bin',
  command          : [arm_objcopy, '--only-section', '.qspi', '-O', 'binary', '@INPUT@', '@OUTPUT@']
)

c47_qspi = custom_target(
  'C47_qspi.bin',
  build_by_default : false,
  input            : [c47_qspi_incorrect_crc],
  output           : 'C47_qspi.bin',
  command          : [bash, modify_crc, forcecrc32, '@INPUT@', '@OUTPUT@']
)

c47_section_sizes = custom_target(
  'C47_section_sizes.txt',
  build_by_default : false,
  capture          : true,
  input            : [c47_elf],
  output           : 'C47_section_sizes.txt',
  command          : [arm_readelf, '-l', '@INPUT@']
)

c47_sizes = run_target(
  'c47_sizes',
  command          : [py3, calc_sizes, 'dmcp5', c47_section_sizes]
)

alias_target('dmcp5', c47_pg5, c47_qspi, c47_sizes)


#
# R47 Build : to be used on DM32 board with R47 keymap.bin plus R47 paper keyboard or R47 overlay/key stickers
#

r47_dmcp_src = files(
  'hal/audio.c',
  'hal/io.c'
)

r47_lib = static_library('libr47',
  build_by_default    : false,
  sources             : decNumber_src + c47_src + c47_dmcp_src + [rasterFontsData_c, softmenuCatalogs_h, constantPointers_ch, vcs_h, version_h],
  c_args              : dmcp_cargs + ['-DCALCMODEL=USER_R47'],
  include_directories : [decNumber_inc, c47_inc, dmcp_inc],
  dependencies        : arm_gmp_dep,
  pic                 : false,
  override_options    : ['debug=false', 'optimization=s']
)

# First pass building the ELF to get the size of the QSPI
r47_pre_elf = executable('R47_pre.elf',
  build_by_default    : false,
  sources             : dmcp_src + [vcs_h, version_h],
  c_args              : dmcp_cargs + ['-DCALCMODEL=USER_R47'],
  link_args           : dmcp_linkargs,
  dependencies        : arm_gmp_dep,
  link_with           : r47_lib,
  include_directories : [c47_inc, dmcp_inc],
  pie                 : false,
  override_options    : ['debug=false', 'optimization=s']
)

r47_pre_qspi_incorrect_crc = custom_target(
  'R47_pre_qspi_incorrect_crc.bin',
  build_by_default : false,
  input            : [r47_pre_elf],
  output           : 'R47_pre_qspi_incorrect_crc.bin',
  command          : [arm_objcopy, '--only-section', '.qspi', '-O', 'binary', '@INPUT@', '@OUTPUT@']
)

r47_pre_qspi = custom_target(
  'R47_pre_qspi.bin',
  build_by_default : false,
  input            : [r47_pre_qspi_incorrect_crc],
  output           : 'R47_pre_qspi.bin',
  command          : [bash, modify_crc, forcecrc32, '@INPUT@', '@OUTPUT@']
)

generated_r47_qspi_crc_h = custom_target(
  'generated_r47_qspi_crc.h',
  build_by_default : false,
  input            : [r47_pre_qspi],
  output           : 'generated_r47_qspi_crc.h',
  command          : [bash, gen_qspi_crc, '@INPUT@', '@OUTPUT@']
)

# Second pass building the ELF including the correct qspi_crc.h
r47_elf = executable('R47.elf',
  build_by_default    : false,
  sources             : dmcp_src + [vcs_h, version_h, generated_qspi_crc_h],
  c_args              : dmcp_cargs + ['-DUSE_GEN_QSPI_CRC'] + ['-DCALCMODEL=USER_R47'],
  link_args           : dmcp_linkargs,
  dependencies        : arm_gmp_dep,
  link_with           : r47_lib,
  include_directories : [c47_inc, dmcp_inc],
  pie                 : false,
  override_options    : ['debug=false', 'optimization=s']
)

r47_flash = custom_target(
  'R47_flash.bin',
  build_by_default : false,
  input            : [r47_elf],
  output           : 'R47_flash.bin',
  command          : [arm_objcopy, '--remove-section', '.qspi', '-O', 'binary', '@INPUT@', '@OUTPUT@']
)

r47_pg5 = custom_target(
  'R47.pg5',
  build_by_default : false,
  input            : [r47_flash],
  output           : 'R47.pg5',
  command          : [bash, add_pgm_chsum, '@INPUT@', '@OUTPUT@']
)

r47_qspi_incorrect_crc = custom_target(
  'R47_qspi_incorrect_crc.bin',
  build_by_default : false,
  input            : [r47_elf],
  output           : 'R47_qspi_incorrect_crc.bin',
  command          : [arm_objcopy, '--only-section', '.qspi', '-O', 'binary', '@INPUT@', '@OUTPUT@']
)

r47_qspi = custom_target(
  'R47_qspi.bin',
  build_by_default : false,
  input            : [r47_qspi_incorrect_crc],
  output           : 'R47_qspi.bin',
  command          : [bash, modify_crc, forcecrc32, '@INPUT@', '@OUTPUT@']
)

r47_section_sizes = custom_target(
  'r47_section_sizes.txt',
  build_by_default : false,
  capture          : true,
  input            : [r47_elf],
  output           : 'r47_section_sizes.txt',
  command          : [arm_readelf, '-l', '@INPUT@']
)

r47_sizes = run_target(
  'r47_sizes',
  command          : [py3, calc_sizes, 'dmcp5', r47_section_sizes]
)

alias_target('dmcp5_r47', r47_pg5, r47_qspi, r47_sizes)
