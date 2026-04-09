local Object = {}

function Object:New()
    local instance = {}
    setmetatable(instance, { __index = self })
    return instance
end

function Object:AddChild(child)
    AddChild_Internal(self, child)
end

function Object:RemoveChild(child)
    RemoveChild_Internal(self, child)
end

function Object:Reparent(newParent)
    Reparent_Internal(self, newParent)
end

function Object:AddTag(newTag)
    AddTag_Internal(self, newTag)
end

function Object:HasTag(tag)
    return HasTag_Internal(self, tag)
end

function Object:GetTags()
    return GetTags_Internal(self)
end

return Object