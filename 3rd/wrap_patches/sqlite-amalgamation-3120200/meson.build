project('sqlite3', 'c')

lib = static_library(
  'sqlite3',
  'sqlite3.c',
  c_args: [
    '-Wno-parentheses-equality',
    '-Wno-unused-value',
    '-Wno-deprecated-declarations',
  ],
)

dep = declare_dependency(
  link_with: lib,
  include_directories: include_directories(
    '.',
  ),
)
