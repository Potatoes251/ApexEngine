local Camera = {}

-- Constructor
function Camera:New(ptr)
    local instance = { __object = ptr }  -- store the C++ pointer/reference
    setmetatable(instance, { __index = self })
    return instance
end

-- Public function: fetch controller for an object
function Camera.Get(obj)
    local ptr = GetCamera_Internal(obj)  -- C++ function returns pointer
    if not ptr then
        return nil
    end
    return Camera:New(ptr)
end

-- Methods
function Camera:GetFront()
    return Camera_GetFront_Internal(self)
end

function Camera:GetRight()
    return Camera_GetRight_Internal(self)
end

function Camera:GetUp()
    return Camera_GetUp_Internal(self)
end

return Camera