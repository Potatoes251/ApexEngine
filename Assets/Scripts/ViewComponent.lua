local Component = require("Component")

local ViewComp = {}
ViewComp.__index = ViewComp
setmetatable(ViewComp, { __index = Component })

-- Constructor
function ViewComp:New(ptr)
    local instance = Component.New(self, ptr)
    return instance
end

-- Public function: fetch controller for an object
function ViewComp.Get(obj)
    local ptr = GetViewComp_Internal(obj)  -- C++ function returns pointer
    if not ptr then
        return nil
    end
    return ViewComp:New(ptr)
end

-- Methods
function ViewComp:SeesTag(tag)
    return ViewComp_SeesTag_Internal(self, tag)
end

function ViewComp:GetClosestWithTag(tag)
    return ViewComp_GetClosestWithTag_Internal(self, tag)
end

return ViewComp