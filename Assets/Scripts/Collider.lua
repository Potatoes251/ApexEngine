local Component = require("Component")

local Collider = {}
Collider.__index = Collider
setmetatable(Collider, { __index = Component })

-- Constructor
function Collider:New(ptr)
    local instance = Component.New(self, ptr)
    return instance
end

-- Public function: fetch controller for an object
function Collider.Get(obj)
    local ptr = GetCollider_Internal(obj)  -- C++ function returns pointer
    if not ptr then
        return nil
    end
    return Collider:New(ptr)
end


return Collider