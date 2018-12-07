subdir('00_ExampleTests')
subdir('ImpactPlayAirExpandSqPointTest01')

UnitTestExecutable = executable(
  'UnitTests',
  'ServiceTesting.cpp',
  'TestLmcpObjectNetworkClient.cpp',
  'UnitTests.cpp',
#
  'RoutePlannerServiceTests.cpp',
#
  dependencies: deps_test,
  cpp_args: cpp_args_test,
  include_directories: inc_test,
  link_with: libs_test,
  link_args: link_args_test,
)

test(
  'UnitTests',
  UnitTestExecutable
)