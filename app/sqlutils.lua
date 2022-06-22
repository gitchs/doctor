#!/usr/bin/env doctor
local re2 = require're2'
local hashlib = require'hashlib'

local libs = {}


local sql_blacklist_re_list = {
    [[(^(?i)select [0-9]+$)]]
}

function libs.strip_comments(sql)
    local sql2 = re2.replace(sql, [[\/\*([^*]|\*[^\/])*\*\/\w*]], '')
    local bs = {sql2:byte(1, -1)}
    for i=#bs,1,-1 do
        -- 删掉右侧多余空格
        -- \t  ==> 9
        -- \n  ==> 10
        -- ' ' ==> 32
        if bs[i]~= 9 and bs[i]~=10 and bs[i]~=32 then
            return sql2:sub(1,i)
        end
    end
    return sql2
end

function libs.sign(sql)
    local sql2 = libs.strip_comments(sql)
    local h = hashlib.xxh64()
    h:update(sql2)
    return h:hexdigest()
end

function libs.in_blacklist(sql)
    for i, re in ipairs(sql_blacklist_re_list) do
        if next(re2.match(sql, re)) ~= nil then
            return true
        end
    end
    return false
end

return libs