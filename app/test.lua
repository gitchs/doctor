#!./doctor
local os = require'os'
local cjson = require'cjson'
local impala = require'impala'
local missing = require'missing'
local logging = require'logging'
local iterators = require'iterators'
local strategies = require'strategies'
local strutils = require'strutils'
local profileutils = require'profileutils'
local dbutils = require'dbutils'
local sqlutils = require'sqlutils'
local optlib = require'optlib'

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
    est_per_host_mem INT DEFAULT -1,
    peak_memory_usage INT DEFAULT -1,
    cluster_memory_admitted VARCHAR(255) NOT NULL,
    hdfs_statics TEXT NOT NULL,
    hash_join_statics TEXT NOT NULL,
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
    conn:setautocommit(0)

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

local function cli_init()
    local retval = {
        avro_file = nil,
        database_file = nil,
        save_profile = false,
    }
    local parser = optlib.Parser.new()
    local it = parser:feed(arg, "f:p?d:h?")
    for is_option, option, value in it do
        assert(is_option, string.format('invalid option "%s"', value))
        if option == 'p' then
            retval.save_profile = true
        elseif option == 'd' then
            retval.database_file = value
        elseif option == 'f' then
            retval.avro_file = value
        elseif option == 'h' then
            print[[./test.lua [-p] -f AVRO_FILE [-d DATABASE_FILE]
    -p                  Save raw profile from avro to sqlite profle table.
    -f AVRO_FILE        Input avro file, only process one value.
    -d DATABASE_FILE    Database file, if missing will generate one automatically.
]]
            os.exit(0)
        end
    end
    return retval
end

local function main()
    local cli_configure = cli_init()
    if cli_configure.avro_file== nil then
        logging.info('no input file')
        os.exit(1)
    end
    local conn = init_db(cli_configure.database_file)
    local sql_counter = 0
    for row in iterators.avro(cli_configure.avro_file) do
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
        -- logging.info('working on query %s', tree:query_id())
        if cli_configure.save_profile then
            dbutils.insert_row(conn, 'profile', row)
            sql_counter = sql_counter + 1
        end

        local eprofile_node = tree2.children[#tree2.children]
        local peak_memory_usage = -1
        if eprofile_node ~= nil and strutils.startswith(eprofile_node.name, 'Execution Profile') then
            local raw_eprofile = getmetatable(eprofile_node).raw
            peak_memory_usage = tonumber(raw_eprofile:info_strings('Peak Memory Usage') or '-1')
        end

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
            est_per_host_mem = tonumber(summary:info_strings('Estimated Per-Host Mem') or '-1'),
            peak_memory_usage = peak_memory_usage,
            hdfs_statics = cjson.encode(strategies.hdfs_statics(tree2)),
            hash_join_statics = cjson.encode(strategies.hash_join_statics(tree2)),
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
        sql_counter = sql_counter + 1
        if sql_counter % 500 == 0 then
            conn:commit()
            sql_counter = 0
        end
        ::next_row::
    end
    conn:commit()
end

if not pcall(debug.getlocal, 4, 1) then
    -- https://stackoverflow.com/questions/49375638/how-to-determine-whether-my-code-is-running-in-a-lua-module
    main()
end
