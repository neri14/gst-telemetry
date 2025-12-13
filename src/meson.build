lib_args = common_args + []

# C sources for GStreamer plugin interface
c_sources = [
  'gsttelemetryplugin.c',
  'gsttelemetry.c',
]

# C++ sources for backend implementation
cpp_sources = [
  'manager.cpp',
  'manager_c_api.cpp',
]

# Headers (for reference, not compiled directly)
headers = [
  'gsttelemetry.h',
  'manager.h',
  'manager_c_api.h',
]

shlib = shared_library('gsttelemetry',
  c_sources + cpp_sources,
  c_args : lib_args,
  cpp_args : lib_args,
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
