local missing = require'missing'
local logging = require'logging'
local libs = {}

libs.Path = {
}
libs.Path.__index = libs.Path

function libs.Path:__tostring()
    return self.path
end

function libs.Path:new(path)
    -- the system path has a length limit, it's safe to use str:byte(1, -1) here
    -- if something is wrong, just let it crash
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

function libs.Path:size()
    local stat = self:stat()
    return stat.st_size
end

function libs.Path:is_directory()
    local stat = self:stat()
    return (stat.st_mode & missing.S_IFMT) == missing.S_IFDIR
end

function libs.Path:stat()
    local stat = missing.stat(self.path)
    if stat.error ~= nil then
        error(stat.error)
        return
    end
    return stat
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

function libs.Path:exists()
    local stat = missing.stat(self.path)
    if stat.errno == nil then
        return true
    end
    if stat.errno ~= 2 then
        logging.error(stat.error)
    end
    return false
end

return libs
