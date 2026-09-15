local Vector3 = require("Vector3")

local Object = {}

-- Factory creation function
function Object.Create(name)
    return CreateObject_Internal(name) 
end

function Object.CreateCube(name)
    return CreateCube_Internal(name)
end

function Object.CreateSphere(name)
    return CreateSphere_Internal(name)
end

function Object:New()
    local instance = {}
    setmetatable(instance, { __index = self })
    return instance
end

function Object:AddComponent(componentTypeName)
    AddComponent_Internal(self, componentTypeName)
end

function Object:SetEnabled(enabled) -- enable/disable the script
    SetEnabled_Internal(self, enabled)
end

function Object:GetPosition()
    return Vector3.New(GetPosition_Internal(self))
end

function Object:GetForward()
    return Vector3.New(GetForward_Internal(self))
end

function Object:SetPosition(x, y, z)
    SetPosition_Internal(self, x, y, z)
end

function Object:SetScale(x, y, z)
    SetScale_Internal(self, x, y, z)
end

function Object:GetName()
    return GetName_Internal(self)
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

function Object:Destroy()
    return Destroy_Internal(self)
end

function Object:DestroyCollectible()
    return DestroyCollectible_Internal(self)
end

return Object