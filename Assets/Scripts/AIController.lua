local Component = require("Component")

local AIController = {}
AIController.__index = AIController
setmetatable(AIController, { __index = Component })

-- Constructor
function AIController:New(ptr)
    local instance = Component.New(self, ptr)
    return instance
end

-- Public function: fetch controller for an object
function AIController.Get(obj)
    local ptr = GetAIController_Internal(obj)  -- C++ function returns pointer
    if not ptr then
        return nil
    end
    return AIController:New(ptr)
end

-- Methods
function AIController:SetTarget(target)
    return AIC_SetTarget_Internal(self, target)
end

function AIController:GetVelocity()
    return CC_GetVelocity_Internal(self)
end

function AIController:IsGrounded()
    return CC_IsGrounded_Internal(self)
end

function AIController:IsInWater()
    return CC_IsInWater_Internal(self)
end

return AIController