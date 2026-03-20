project(
  'nstt7',
  'cpp',
  default_options: [
    'cpp_std=c++23',
    'warning_level=3',
  ],
)

tests = executable('tests', ['emulator.cpp'], dependencies: [dependency('gtest_main')])
test('tests', tests)
