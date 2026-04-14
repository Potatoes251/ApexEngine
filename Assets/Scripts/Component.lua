local Component = {}
Component.__index = Component

-- Constructor
function Component:New(ptr)
    local instance = { __object = ptr }
    setmetatable(instance, self)
    return instance
end


-- Methods
function Component:GetOwner()
    return GetOwner_Internal(self)
end

return Component