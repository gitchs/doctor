#!/usr/bin/env doctor
local os = require'os'
local math = require'math'
local cjson = require'cjson'
local logging = require'logging'
local profileutils = require'profileutils'
local sqlite3 = require'luasql.sqlite3'.sqlite3
local strutils = require'strutils'

local function mem2bytes(m)
    if m == nil or m == "" then
        return 0
    end

    if strutils.endswith(m, ' GB') then
        return math.floor(tonumber(m:sub(1, #m - 3)) * 1024 * 1024 * 1024)
    end
    if strutils.endswith(m, ' MB') then
        return math.floor(tonumber(m:sub(1, #m - 3)) * 1024 * 1024)
    end
    if strutils.endswith(m, ' KB') then
        return math.floor(tonumber(m:sub(1, #m - 3)) * 1024)
    end
    return math.floor(tonumber(m))
end

local function db_it(db_filename)
    local env = sqlite3()
    local conn = env:connect(db_filename)

    local cursor = conn:execute([[select max(CAST(STRFTIME('%s', DATETIME(end_time)) AS INT)) FROM m2]])
    assert(cursor ~= nil)
    local max_end_time = cursor:fetch()

    local sql = [[SELECT
	query_id,
	CAST(STRFTIME('%s', DATETIME(start_time)) AS INT) AS start_time,
	CAST(STRFTIME('%s', DATETIME(start_time, '+' || CAST(admission_wait/1000 AS STRING) || ' seconds')) AS INT) AS exec_time,
	CAST(STRFTIME('%s', DATETIME(end_time)) AS INT) AS end_time,
	CAST(
		strftime('%s', end_time) AS INT)
			-  CAST(strftime('%s', DATETIME(start_time, '+' || CAST(admission_wait/1000 AS STRING) || ' seconds'))
	AS INT) AS exec_duration,
	query_state,
	query_status,
    cluster_memory_admitted,
	admission_wait/1000 AS admission_wait_sec
FROM
	m2
WHERE
resource_pool = 'root.default' AND
NOT query_status LIKE 'Admission for query exceeded timeout%'
AND exec_duration > 5
ORDER BY exec_time
]]
    local err = nil
    cursor, err = conn:execute(sql)
    if cursor == nil then
        print(err)
    end
    assert(cursor ~= nil, err)
    if cursor == nil then
        return nil
    end
    local is_done = false
    local names = cursor:getcolnames()
    return max_end_time, function ()
        if is_done then
            return nil
        end
        local row = {cursor:fetch()}
        if row == nil or row[1] == nil then
            is_done = true
            return nil
        end
        local retval = {}
        for i, name in ipairs(names) do
            retval[name] = row[i]
        end
        if retval.cluster_memory_admitted ~= nil then
            retval.cluster_memory_admitted = mem2bytes(retval.cluster_memory_admitted)
        end
        return retval
    end
end

local Status = {}

Status['__index'] = Status

function Status.new(init_row, window_end)
    if window_end == nil then
        window_end = os.time()
    end
    local o = {
        now = init_row.exec_time,
        window_end = window_end,
        queries = {
            init_row,
        },
    }
    setmetatable(o, Status)
    return o
end

function Status:done()
    return self.now > self.window_end
end

function Status:next_ticktock()
    self.now = self.now + 1
    local queries = {}
    -- shift outdate queries
    for _, query in ipairs(self.queries) do
        if self.now < query.end_time then
            table.insert(queries, query)
        end
    end
    self.queries = queries
    return self.now
end

function Status:update(row)
    assert(row ~= nil)
    if row.exec_time > self.now then
        return false
    end
    table.insert(self.queries, row)
    return true
end

function Status:report()
    local mem_sum = 0
    for i =1, #self.queries do
        local query = self.queries[i]
        mem_sum = mem_sum + query.cluster_memory_admitted
    end
    mem_sum = mem_sum / 1024 / 1024 / 1024
    return string.format('%s,%s,%s', self.now, #self.queries, mem_sum)
end

local function main()
    -- 目前仅绘制了预估的内存占用；
    -- TODO: 统计实际的内存占用
    local db_filename = arg[1]
    if db_filename == nil then
        logging.error('no input database')
        os.exit(1)
    end
    local max_end_time, it = db_it(db_filename)
    logging.info('max_end_time %s, it %s', max_end_time, it)
    assert(it ~= nil, 'failed to open database or execute sql')
    local init_row = it()
    local status = Status.new(init_row, max_end_time + 600)
    local last_row = nil
    while not status:done() do
        local report_line = status:report()
        print(report_line)
        status:next_ticktock()
        if last_row == nil then
            last_row = it()
        end
        while true do
            if last_row == nil or not status:update(last_row) then
                break
            end
            last_row = it()
        end
    end
end

if not pcall(debug.getlocal, 4, 1) then
    main()
end
