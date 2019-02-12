project('zlib', 'c')

c_args_zlib = [
  '-Wno-implicit-function-declaration',
  '-Wno-shift-negative-value',
]

lib = static_library(
  'zlib',
  [
    'adler32.c',
    'compress.c',
    'crc32.c',
    'deflate.c',
    'gzclose.c',
    'gzlib.c',
    'gzread.c',
    'gzwrite.c',
    'infback.c',
    'inffast.c',
    'inflate.c',
    'inftrees.c',
    'trees.c',
    'uncompr.c',
    'zutil.c',
  ],
  c_args: c_args_zlib,
)

dep = declare_dependency(
  link_with: lib,
  include_directories: include_directories(
    '.',
  ),
)
