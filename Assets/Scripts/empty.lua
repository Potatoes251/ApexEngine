local Empty = {}

function Empty:New()
    local instance = {}
    setmetatable(instance, { __index = self })
    return instance
end

return Empty