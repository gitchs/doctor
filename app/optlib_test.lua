#!./doctor
local optlib = require'optlib'
local define = optlib.define


define('--port', '-p', {default='10086', type=optlib.tointeger})
define('--address', {default='localhost'})
define('--verbose', '-v', {nargs=0})

local args = {
  '-p', '12345',
  '--address', '0.0.0.0',
  '--verbose=true',
}
optlib.parse(args)

print(optlib.get('address'))
local port = optlib.get('port')
print(port, type(port))
