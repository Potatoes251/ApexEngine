local M = {}

function M:New()
    local instance = {
        speed = 5.0,
        health = 10,
        active = true,
        name = "Enemy"
    }

    instance.__exposed = {
        "speed",
        "health",
        "active"
    }

    setmetatable(instance, { __index = self })
    return instance
end

return M