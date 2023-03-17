#!./doctor
-- extract operator counters from raw thrift profile
-- start with '-h' to check how to use it.

local cjson = require'cjson'
local impala = require'impala'
local io = require'io'
local os = require'os'
local profileutils = require'profileutils'
local optlib = require'optlib'
local define = optlib.define


define('--help', '-h', {nargs=0})
define('--file', '-f')
define('--id', '-i')


optlib.parse(arg)

local filename = optlib.get('file')
local target_oid = optlib.get('id')

if optlib.get('help') or not filename or not target_oid then
  print[[list_op_counters -f PROFILE_FILE -i OPERATOR_ID
]]
  os.exit(0)
end



local raw = io.open(filename):read()
local ok, err, tree = impala.parse_profile(raw)
assert(ok and err == nil)
local tree2 = profileutils.build_tree(tree)
local gops = profileutils.group_operators(tree2)


for oid, ops in pairs(gops) do
  if oid ~= target_oid then
    goto NEXT_OPS
  end

  local rows = {}
  for _, op in ipairs(ops) do
    local row = {
      host = op.host,
      name = op.name,
    }
    for k, v in pairs(op.counters) do
      row[k] = v
    end
    table.insert(rows, row)
  end
  print(cjson.encode(rows))
  break

  ::NEXT_OPS::
end


