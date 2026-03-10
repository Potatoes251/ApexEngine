local M = {}

function M:New()
    local instance = {}
    setmetatable(instance, { __index = self })
    return instance
end

function M:Start()
    print("Start called")
end

function M:Update(dt)
    print("Update 2")
end

return M