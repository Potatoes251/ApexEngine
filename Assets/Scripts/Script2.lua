local M = {}

function M:New()
    local instance = {}
    setmetatable(instance, { __index = self })

    instance.speed = 10
    instance.name = "Enemy"
    instance.active = true

    return instance
end

return M