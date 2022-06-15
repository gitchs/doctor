#!/usr/bin/env doctor
local os = require'os'
local impala = require'impala'
local logging = require'logging'
local iterators = require'iterators'
local strategies = require'strategies'
local dbutils = require'dbutils'

local init_statements = {
    -- [[DROP TABLE IF EXISTS m2]],
    [[
CREATE TABLE IF NOT EXISTS m2(
    query_id VARCHAR(255) NOT NULL,
    query_type VARCHAR(64) NOT NULL,
    query_state VARCHAR(64) NOT NULL,
    query_status TEXT NOT NULL,
    is_slow TINYINT NOT NULL,
    has_skew_ops TINYINT NOT NULL,
    start_time datetime NOT NULL,
    end_time DATETIME NOT NULL,
    duration INTEGER NOT NULL,
    result LONGTEXT NOT NULL
)
]],
    [[
CREATE UNIQUE INDEX IF NOT EXISTS idx_m2_query_id ON m2(query_id)
]],
}

function init_db()
    local database = 'test'
    local username = nil
    local password = nil
    local host = nil
    local port = nil
    local unix_socket = '/tmp/mysql.sock'
    local env = require'luasql.mysql'.mysql()
    local conn = env:connect(database, username, password, host, port, unix_socket)
    for _, init_statement in ipairs(init_statements) do
        local ok, err = conn:execute(init_statement)
        if ok == nil then
            logging.error(err)
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
    local conn = init_db()
    for row in iterators.avro(filename) do
        local ok, err, tree = impala.parse_profile(row.profile)
        local result = strategies.test_profile(tree)
        local summary = tree:node_at(2)
        local row = {
            query_id = tree:query_id(),
            query_type = summary:info_strings('Query Type'),
            query_state = summary:info_strings('Query State'),
            query_status = summary:info_strings('Query Status'),
            start_time = function() return string.format("'%s'",summary:info_strings('Start Time')) end,
            end_time = function() return string.format("'%s'",summary:info_strings('End Time')) end,
            duration = tonumber(summary:info_strings('Duration(ms)')),
            is_slow = result.is_slow,
            has_skew_ops = false,
            result = '',
        }
        if result.skew_ops ~= nil and #result.skew_ops > 0 then
            row.has_skew_ops = true
        end
        if row.has_skew_ops then
            row.result = table.concat(result.skew_ops, '|')
        end
        dbutils.insert_row(conn, 'm2', row)
    end
end

main()
