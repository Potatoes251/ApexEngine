local M = {}

function M:New()
    local instance = {}
    setmetatable(instance, { __index = self })
    return instance
end

function M:AddChild(child)
    AddChild_Internal(self, child)
end

function M:RemoveChild(child)
    RemoveChild_Internal(self, child)
end

function M:Reparent(newParent)
    Reparent_Internal(self, newParent)
end

return M