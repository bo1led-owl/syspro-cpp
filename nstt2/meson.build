project(
  'nstt2',
  'cpp',
  default_options: [
    'cpp_std=c++23',
    'warning_level=3',
  ],
)

tests = executable('tests', ['tests.cpp'], dependencies: [dependency('gtest_main')])
test('tests', tests)
