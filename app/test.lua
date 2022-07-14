#!/usr/bin/env doctor
local os = require'os'
local cjson = require'cjson'
local impala = require'impala'
local missing = require'missing'
local logging = require'logging'
local iterators = require'iterators'
local strategies = require'strategies'
local profileutils = require'profileutils'
local dbutils = require'dbutils'
local sqlutils = require'sqlutils'

local init_statements = {
    -- [[DROP TABLE IF EXISTS m2]],
    [[
CREATE TABLE IF NOT EXISTS m2(
    query_id VARCHAR(255) NOT NULL,
    query_type VARCHAR(64) NOT NULL,
    query_state VARCHAR(64) NOT NULL,
    resource_pool VARCHAR(255) NOT NULL,
    resource_pool_size VARCHAR(64) NOT NULL,
    query_status TEXT NOT NULL,
    coordinator VARCHAR(64) NOT NULL,
    rows_produced INT NOT NULL,
    cluster_memory_admitted VARCHAR(255) NOT NULL,
    hdfs_statics TEXT NOT NULL,
    is_slow TINYINT NOT NULL,
    has_skew_ops TINYINT NOT NULL,
    start_time datetime NOT NULL,
    end_time DATETIME NOT NULL,
    duration INTEGER NOT NULL,
    admission_wait LONG NOT NULL,
    result LONGTEXT NOT NULL,
    `sql_sign` VARCHAR(32) NOT NULL,
    `sql` LONGTEXT NOT NULL
)
]],
    [[ CREATE UNIQUE INDEX IF NOT EXISTS idx_m2_query_id ON m2(query_id) ]],
    [[ CREATE TABLE IF NOT EXISTS profile(
    query_id VARCHAR(255) NOT NULL,
    profile LONGTEXT NOT NULL
);]],
    [[ CREATE UNIQUE INDEX IF NOT EXISTS idx_profile_query_id ON profile(query_id) ]],
}

local function init_db(database)
    -- local database = 'test'
    -- local username = nil
    -- local password = nil
    -- local host = nil
    -- local port = nil
    -- local unix_socket = '/tmp/mysql.sock'
    -- local env = require'luasql.mysql'.mysql()
    -- local conn = env:connect(database, username, password, host, port, unix_socket)

    if database == nil then
        database = string.format('sqlite-%d.db', missing.getpid())
        if os.getenv('DB_ROOT') ~= nil then
            database = string.format('%s/%s', os.getenv('DB_ROOT'), database)
        end
        logging.info('arg database is nil, the generated one is "%s"', database)
    end
    local env = require'luasql.sqlite3'.sqlite3()
    local conn = env:connect(database)

    for _, init_statement in ipairs(init_statements) do
        local ok, err = conn:execute(init_statement)
        if ok == nil then
            logging.error(err)
            os.exit(1)
        end
    end
    conn:commit()
    return conn, env
end

local function main()
    local filename = arg[1]
    if filename == nil then
        logging.info('no input file')
        os.exit(1)
    end
    local database_filename = arg[2]
    local conn = init_db(database_filename)
    for row in iterators.avro(filename) do
        local ok, err, tree = impala.parse_profile(row.profile)
        assert(ok and err == nil)
        local tree2 = profileutils.build_tree(tree)
        local result = strategies.test_profile(tree2)
        local summary = tree:node_at(2)
        local query_type = summary:info_strings('Query Type')
        if query_type ~= 'QUERY' and query_type ~= 'DML' then
            goto next_row
        end
        local sql = summary:info_strings('Sql Statement')
        if sqlutils.in_blacklist(sql) then
            goto next_row
        end
        -- dbutils.insert_row(conn, 'profile', row)
        local db_row = {
            query_id = tree:query_id(),
            query_type = query_type,
            query_state = summary:info_strings('Query State'),
            resource_pool = summary:info_strings('Request Pool') or '',
            resource_pool_size = summary:info_strings('Request Pool Size') or '',
            query_status = summary:info_strings('Query Status'),
            start_time = function() return string.format("'%s'",summary:info_strings('Start Time')) end,
            end_time = function() return string.format("'%s'",summary:info_strings('End Time')) end,
            duration = tonumber(summary:info_strings('Duration(ms)') or '-1'),
            admission_wait = tonumber(summary:info_strings('Admission Wait') or '-1'),
            cluster_memory_admitted = summary:info_strings('Cluster Memory Admitted') or '',
            hdfs_statics = cjson.encode(strategies.hdfs_statics(tree2)),
            is_slow = result.is_slow,
            has_skew_ops = false,
            coordinator = summary:info_strings('Coordinator'),
            rows_produced = tonumber(summary:info_strings('Rows Produced') or '-1'),
            result = '',
            sql_sign = sqlutils.sign(sql),
            sql = sql,
        }
        if result.skew_ops ~= nil and #result.skew_ops > 0 then
            db_row.has_skew_ops = true
        end
        if db_row.has_skew_ops then
            db_row.result = table.concat(result.skew_ops, '\n')
        end
        dbutils.insert_row(conn, 'm2', db_row)
        ::next_row::
    end
end

if not pcall(debug.getlocal, 4, 1) then
    -- https://stackoverflow.com/questions/49375638/how-to-determine-whether-my-code-is-running-in-a-lua-module
    main()
end
