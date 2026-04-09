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

function M:AddTag(newTag)
    AddTag_Internal(self, newTag)
end

function M:HasTag(tag)
    return HasTag_Internal(self, tag)
end

function M:GetTags()
    return GetTags_Internal(self)
end

return M