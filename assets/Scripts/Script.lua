local M = {}

function M:New()
    local instance = {}
    setmetatable(instance, { __index = self })

    instance.transform = {}

    local inst = instance
    function instance.transform:SetPosition(x, y, z)
        SetPosition_Internal(inst, x, y, z)
    end

    return instance
end

function M:Start()
    print("Start called")
end

function M:Update(dt)
    print("Update 1")
    self.transform:SetPosition(0, math.random(0,5), 0)
end

return M