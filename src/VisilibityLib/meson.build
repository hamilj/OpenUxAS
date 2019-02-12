cpp_args_visilibity = cpp_args

cpp_args_visilibity += [
  '-Wno-unused-parameter',
]

if cpp.get_id() == 'gcc'
  cpp_args_visilibity += [
    '-Wno-strict-overflow',
    '-Wno-maybe-uninitialized',
  ]
endif

lib_visilibity = static_library(
  'visilibity',
  [
    'visilibity.cpp',
  ],
  dependencies: [
    dep_boost,
  ],
  cpp_args: cpp_args_visilibity,
)
