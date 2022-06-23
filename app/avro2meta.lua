#!/usr/bin/env doctor
local math = require'math'
local strutils = require'strutils'
local pathlib = require'pathlib'
local logging = require'logging'
local iterators = require'iterators'
local sqlutils = require'sqlutils'
local hdfsutils = require'hdfsutils'
local limits = require'limits'
local cjson = require'cjson'
local impala = require'impala'

local function init_db()
    local init_statements = {
        -- [[drop table if exists meta]],
        [[create table if not exists meta(
query_id VARCHAR(255) NOT NULL,
query_type VARCHAR(64) NOT NULL,
query_state VARCHAR(255) NOT NULL,
coordinator VARCHAR(255) NOT NULL,
cluster_memory VARCHAR(255) NOT NULL,
session_type VARCHAR(64) NOT NULL,
`sql_sign` VARCHAR(32) NOT NULL,
start_time DATETIME NOT NULL,
end_time DATETIME NOT NULL,
duration LONG NOT NULL,
admission_result VARCHAR(255) NOT NULL,
admission_wait LONG NOT NULL,
admission_timeout TINYINT NOT NULL,
query_status MEDIUMTEXT NOT NULL,
hdfs_sum_io LONG NOT NULL,
hdfs_min_total_time LONG NOT NULL, -- should convert it from ns to ms
hdfs_max_total_time LONG NOT NULL, -- should convert it from ns to ms
hdfs_skew TINYINT NOT NULL, -- if empty hdfs io, it will always be false
hdfs_skew_counters MEDIUMTEXT DEFAULT NULL,
avro_filename TEXT,
`sql` LONGTEXT NOT NULL
)
DEFAULT CHARACTER SET utf8mb4
DEFAULT COLLATE utf8mb4_general_ci;
        ]],
        [[create index if not exists idx_meta_query_id ON meta(query_id)]],
        [[create index if not exists idx_meta_avro_filename on meta(avro_filename)]],

        [[
CREATE TABLE IF NOT EXISTS hdfs_counters(
    query_id VARCHAR(255) NOT NULL,
    fragment VARCHAR(255) NOT NULL,
    instance VARCHAR(255) NOT NULL,
    `host` VARCHAR(255) NOT NULL,
    counters LONGTEXT NOT NULL
)
]],
        [[create index if not exists idx_hdfs_counters_query_id ON hdfs_counters(query_id)]],

    }
    local env = require'luasql.mysql'.mysql()
    local username = nil
    local password = nil
    local host = nil
    local port = nil
    local unix_socket = '/tmp/mysql.sock'
    local conn, err = env:connect('test', username, password, host, port, unix_socket)
    assert(conn ~= nil, err or 'failed to create connection')

    for i, init_statement in ipairs(init_statements) do
        local ok, err = conn:execute(init_statement)
        if ok == nil then
            print(err)
        end
    end
    conn:commit()
    return env, conn
end

local function process_profile(conn, f, tree)
    local query_id = tree:query_id()
    local summary = tree:node_at(2)
    local query_type = summary:info_strings('Query Type')
    if query_type ~= 'QUERY' and query_type ~= 'DML' then
        return
    end
    local sql = summary:info_strings('Sql Statement')
    if sqlutils.in_blacklist(sql) then
        return
    end

    local query_state = summary:info_strings('Query State') or ''
    local query_status = summary:info_strings('Query Status') or ''
    local start_time = summary:info_strings('Start Time'):sub(1,19)
    local end_time = summary:info_strings('End Time'):sub(1,19)
    local duration = tonumber(summary:info_strings('Duration(ms)'))
    local admission_result = summary:info_strings('Admission result')
    local admission_wait = tonumber(summary:info_strings('Admission Wait') or 0)
    local cluster_memory = summary:info_strings('Cluster Memory Admitted') or ''
    local coordinator = summary:info_strings('Coordinator')
    local admission_timeout = 0
    if query_state == 'EXCEPTION' then
        -- Admission for query exceeded timeout 720000ms in pool root.default. Queued reason: queue is not empty (size 4); queued queries are executed first.
        if strutils.startswith(query_status, 'Admission for query exceeded timeout ') then
            admission_timeout = 1
        end
    end

    -- 注意: 异常结束的查询，统计的COUNTER不准确，不能作为参考

    local hdfs_counters = hdfsutils.list_hdfs_node_counters(tree)
    local skew_counters = {}

    local hdfs_sum_io = 0
    local hdfs_max_total_time = 0
    local hdfs_min_total_time = limits.LLONG_MAX
    for _, counter in ipairs(hdfs_counters) do
        local total_time = counter['TotalTime'] or 0
        if total_time > hdfs_max_total_time then
            hdfs_max_total_time = total_time
        end
        if total_time < hdfs_min_total_time then
            hdfs_min_total_time = total_time
        end
        hdfs_sum_io = hdfs_sum_io + (counter['BytesRead'] or 0)
    end
    hdfs_max_total_time = math.floor(hdfs_max_total_time/1e6) -- convert from ns to ms
    if hdfs_min_total_time == limits.LLONG_MAX then
        hdfs_min_total_time = 0
    else
        hdfs_min_total_time = math.floor(hdfs_min_total_time/1e6)
    end

    if hdfs_sum_io > 0 then
        skew_counters = hdfsutils.skew_counters(hdfs_counters)
    end

    for _, counter in ipairs(hdfs_counters) do
        local counter_record = {
            query_id = query_id,
            host = counter.instance:gmatch('host=([a-z0-9.:]+)')(),
            fragment = counter.fragment,
            instance = counter.instance,
            counters = cjson.encode(counter),
        }
        require'dbutils'.insert_row(conn, 'hdfs_counters', counter_record)
    end

    local row = {
        query_id = query_id,
        query_type = query_type,
        query_state = query_state,
        coordinator = coordinator,
        cluster_memory = cluster_memory,
        start_time = start_time,
        end_time = end_time,
        duration = duration,
        admission_result = admission_result,
        admission_wait = admission_wait,
        admission_timeout = admission_timeout,
        query_status = query_status,
        avro_filename = f.name,

        hdfs_sum_io = hdfs_sum_io,
        hdfs_max_total_time = hdfs_max_total_time,
        hdfs_min_total_time = hdfs_min_total_time,
        hdfs_skew = next(skew_counters) ~= nil,
        hdfs_skew_counters = cjson.encode(skew_counters),

        session_type = summary:info_strings('Session Type') or '',
        sql_sign = sqlutils.sign(sql),
        sql = sql,
    }
    require'dbutils'.insert_row(conn, 'meta', row)
end

local function process_file(f)
    local env, conn = init_db()
    for row in iterators.avro(f.path) do
        local ok, err, tree = impala.parse_profile(row.profile)
        if not ok then
            logging.error('failed to parse query %s from avro %s', row.query_id, f)
            goto continue_parse_profile
        end
        process_profile(conn, f, tree)
        ::continue_parse_profile::
    end
end

local function main()
    local f = arg[1]
    if f == nil then
        logging.error('no input file')
        os.exit(1)
    end
    logging.info('handle file %s', f)
    process_file(pathlib.Path:new(f))
end

local function main2()
    local env, conn = init_db()
    local root = pathlib.Path:new('/Volumes/HKVSN/profile_t38904/')
    for f in root:listdir() do
        if not strutils.endswith(f.name, '.txt') then
            goto main2_continue
        end
        local fd = io.open(f.path, 'r')
        assert(fd ~= nil)
        logging.info('handle file %s', f)
        local raw = fd:read('*all')
        local ok, err, tree = impala.parse_profile(raw)
        if not ok then
            logging.error('error %s', err)
        end
        assert(ok)
        process_profile(conn, f, tree)
        ::main2_continue::
    end
end

main()
-- main2()
