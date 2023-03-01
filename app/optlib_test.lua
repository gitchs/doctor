#!./doctor
local optlib = require'optlib'


local parser = optlib.Parser.new()
local FLAGS_FILENAME = 'INPUT_FILENAME'
local FLAGS_VERBOSE_COUNTERS = 0
local args = {'-f', FLAGS_FILENAME, '-v'}

for is_option, option, value in parser:feed(args, "f:v") do
  assert(is_option, 'get unknown option')
  if option == 'f' then
    assert(value == FLAGS_FILENAME, '-f value does not match')
    goto NEXT
  end

  if option == 'v' then
    assert(FLAGS_VERBOSE_COUNTERS == 0, 'FLAGS_VERBOSE_COUNTERS should be 0 now')
    FLAGS_VERBOSE_COUNTERS = FLAGS_VERBOSE_COUNTERS + 1
    goto NEXT
  end

  ::NEXT::
end

assert(FLAGS_VERBOSE_COUNTERS == 1, 'FLAGS_VERBOSE_COUNTERS should be 1 now')

