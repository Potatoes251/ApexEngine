local Component = require("Component")

local Camera = {}
Camera.__index = Camera
setmetatable(Camera, { __index = Component })

-- Constructor
function Camera:New(ptr)
    local instance = Component.New(self, ptr)
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
function Camera:SetMain()
    return Camera_SetMain_Internal(self)
end

function Camera:SetYaw(degree)
    return Camera_SetYaw_Internal(self, degree)
end

function Camera:SetPitch(degree)
    return Camera_SetPitch_Internal(self, degree)
end

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