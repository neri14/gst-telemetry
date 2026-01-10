lib_args = common_args + []

# C sources for GStreamer plugin interface
c_sources = [
  'gsttelemetryplugin.c',
  'gsttelemetry.c',
  'buffer_pool.c',
]

# C++ sources for telemetry backend
cpp_sources = [
]

# Headers (for reference, not compiled directly)
headers = [
  'gsttelemetry.h',
  'buffer_pool.h',
]

# Include backend subdirectory
subdir('api')
subdir('backend')
subdir('trace')

shlib = shared_library('gsttelemetry',
  c_sources + cpp_sources,
  c_args : lib_args,
  cpp_args : lib_args,
  include_directories: [configinc, include_directories('.')],
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
