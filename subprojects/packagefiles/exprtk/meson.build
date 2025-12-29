project('exprtk', 'cpp',
  version: '0.0.3',
  license: 'MIT')

# ExprTk is a header-only library
exprtk_inc = include_directories('.')

exprtk_dep = declare_dependency(
  include_directories: exprtk_inc,
  version: meson.project_version()
)

# Make the dependency available
meson.override_dependency('exprtk', exprtk_dep)
