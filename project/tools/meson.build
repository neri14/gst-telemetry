exe_args = common_args + []

# sources used to compile this program
tool_sources = [
  'gsttelemetry.c',
]

executable('gsttelemetry',
  tool_sources,
  install: true,
  c_args : exe_args,
  include_directories: [configinc],
  dependencies : tool_deps,
)
