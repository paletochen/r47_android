c47_gtk_src = files(
  'c47-gtk.c',
  'gtkGui.c',
  'hal/audio.c',
  'hal/gui.c',
  'hal/lcd.c',
  'hal/io.c')

override_opts = ['b_coverage=false']
cflags        = []
linkargs      = []
c47_gtk_rc    = []
r47_gtk_rc    = []

if get_option('buildtype').startswith('release')
  override_opts += ['b_lto=true']
else
  cflags        += ['-fstack-protector-strong']
  linkargs      += ['-fstack-protector-strong']
  # Sanitizers not supported on MinGW
  if (host_machine.system() == 'linux' or host_machine.system() == 'darwin')
    override_opts += ['b_sanitize=address,undefined']
  endif
endif

if (target_machine.system() == 'windows')
  windows = import('windows')
  c47_gtk_rc += windows.compile_resources('c47-gtk.rc', depend_files: [version_h], depends: [vcs_h], include_directories: [c47_inc])
  r47_gtk_rc += windows.compile_resources('r47-gtk.rc', depend_files: [version_h], depends: [vcs_h], include_directories: [c47_inc])
endif

#
# C47 Simulator
#

deps = [gtk_dep, gmp_dep, m_dep, pulse_dep]
_c_args = []

if host_machine.system() == 'linux'
  _c_args += ['-D_GNU_SOURCE']
endif

have_dladdr = false
cc = meson.get_compiler('c')

if cc.has_function('dladdr', prefix : '#include <dlfcn.h>')
  have_dladdr = true
endif

if not have_dladdr and host_machine.system() == 'linux'
  libdl = cc.find_library('dl', required : false)
  if libdl.found() and cc.has_function('dladdr',
      prefix : '#include <dlfcn.h>',
      dependencies : [libdl])
    deps += libdl
    have_dladdr = true
  endif
endif

if have_dladdr
  _c_args += ['-DHAVE_DLADDR=1']
endif

sim = executable('c47',
  sources             : decNumber_src + c47_src + c47_gtk_src + c47_gtk_rc + [rasterFontsData_c, softmenuCatalogs_h, constantPointers_ch, vcs_h, version_h],
  c_args              : _c_args + ['-DCALCMODEL=USER_C47'],
  c_pch               : 'pch/c47-gtk_pch.h',
  include_directories : [decNumber_inc, c47_inc],
  dependencies        : deps,
  native              : true,
  override_options    : ['b_lto=true'],
  win_subsystem       : 'windows'
)

alias_target('sim', sim)

#
# R47 Simulator
#

simr47 = executable('r47',
  sources             : decNumber_src + c47_src + c47_gtk_src + r47_gtk_rc + [rasterFontsData_c, softmenuCatalogs_h, constantPointers_ch, vcs_h, version_h],
  c_args              : _c_args + ['-DCALCMODEL=USER_R47'],
  c_pch               : 'pch/c47-gtk_pch.h',
  include_directories : [decNumber_inc, c47_inc],
  dependencies        : deps,
  native              : true,
  override_options    : ['b_lto=true'],
  win_subsystem       : 'windows'
)

alias_target('simr47', simr47)
