local M = {}

function M:New()
    local instance = {}
    setmetatable(instance, { __index = self })

    function instance:AddChild(child)
        AddChild_Internal(self, child)
    end

    function instance:RemoveChild(child)
        RemoveChild_Internal(self, child)
    end

    return instance
end

function M:Start()
    print("Parent Start")

    self:AddChild(self.other)

    self:RemoveChild(self.other);
end

function M:Update(dt)
end

return M