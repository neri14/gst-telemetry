lib_args = common_args + []

# sources used to compile this plug-in
sources = [
  'gsttelemetryplugin.c',
  'gsttelemetry.c',
  'gsttelemetry.h'
]

shlib = shared_library('gsttelemetry',
  sources,
  c_args : lib_args,
  include_directories: [configinc],
  dependencies : plugin_deps,
  gnu_symbol_visibility : 'hidden',
  install : true,
  install_dir : plugins_install_dir,
)

# Make this library usable as a Meson subproject.
gst_telemetry_dep = declare_dependency(
  include_directories: include_directories('.'),
  link_with : shlib)

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name : 'gst-telemetry',
  filebase : 'gst-telemetry',
  description : 'Meson sample project.',
  subdirs : 'src',
  libraries : shlib,
  version : '"@0@"'.format(meson.project_version()),
)
