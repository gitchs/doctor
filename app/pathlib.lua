local missing = require'missing'
local libs = {}

libs.Path = {
}
libs.Path.__index = libs.Path


function libs.Path:__tostring()
    return self.path
end

function libs.Path:new(path)
    local bs = {path:byte(1,-1)}
    local basename = nil
    local name = nil
    for i=#bs,1,-1 do
        if bs[i] == 47 then -- 47 ==> '/'
            name = path:sub(i+1, -1)
            break
        end
    end
    if name == nil or name == '' then
        name = path
    end
    bs = {name:byte(1,-1)}
    for i=#bs,1,-1 do
        if bs[i] == 46 then
            basename = name:sub(1, i-1)
            break
        end
    end
    if basename == nil or basename == '' then
        basename = name
    end
    local o = {
        basename = basename,
        name = name,
        path = path,
    }
    setmetatable(o, self)
    return o
end


function libs.Path:listdir()
    local ok, files = missing.readdir(self.path)
    if not ok then
        return nil
    end
    local index = 1
    return function()
        local f = files[index]
        if f == nil then
            return nil
        end
        index = index + 1
        return libs.Path:new(string.format('%s/%s', self.path, f))
    end
end

return libs
