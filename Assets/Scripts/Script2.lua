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

function M:OnTriggerEnter(dt)
    print("Enter")
end

function M:OnTriggerExit(dt)
    print("Exit")
end

return M