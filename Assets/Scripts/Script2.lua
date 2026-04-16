local M = {}

function M:New()
    local instance = {}
    setmetatable(instance, { __index = self })

    instance.speed = 10
    instance.name = "Enemy"
    instance.active = true

    return instance
end

function M:OnTriggerEnter()
    print("Enter")
end

function M:OnTriggerExit()
    print("Exit")
end

return M