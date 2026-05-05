sphinx =  find_program('sphinx-build',   required: false)
doxygen = find_program('doxygen',        required: false)
breathe = find_program('breathe-apidoc', required: false)

if sphinx.found() and doxygen.found() and breathe.found()
  docs= custom_target(
    'docs',
    output  : ['index.html'],
    command : [sphinx, '-M', 'html', meson.current_source_dir(), meson.current_build_dir()])
  alias_target('docs', docs)
else
  message('Sphinx, Doxygen, or Breathe not found so cannot produce code documentation')
endif
