local libs = {}

function libs.insert_row(conn, tname, row)
    local p0 = {}
    local p1 = {}
    for key, val in pairs(row) do
        table.insert(p0, string.format('`%s`', key))
        if type(val) == 'boolean' then
            if val then
                table.insert(p1, '1')
            else
                table.insert(p1, '0')
            end
        elseif type(val) == 'string' then
            table.insert(p1, string.format([['%s']], conn:escape(val)))
        elseif type(val) == 'number' then
            table.insert(p1, tostring(val))
        elseif type(val) == 'function' then
            table.insert(p1, val())
        end
    end
    local sql = string.format(
        [[insert into %s(%s) VALUES(%s)]],
        tname,
        table.concat(p0, ','),
        table.concat(p1, ',')
    )
    local cursor, err = conn:execute(sql)
    return cursor, err
end

return libs
