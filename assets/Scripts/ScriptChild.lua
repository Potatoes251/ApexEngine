local M = {}

function M:New()
    local instance = {}
    setmetatable(instance, { __index = self })

    return instance
end

function M:Start()
    print("Child Start")
end

function M:Update(dt)
end

return M