local Component = require("Component")

local CharacterController = {}
CharacterController.__index = CharacterController
setmetatable(CharacterController, { __index = Component })

-- Constructor
function CharacterController:New(ptr)
    local instance = Component.New(self, ptr)
    return instance
end

-- Public function: fetch controller for an object
function CharacterController.Get(obj)
    local ptr = GetCharacterController_Internal(obj)  -- C++ function returns pointer
    if not ptr then
        return nil
    end
    return CharacterController:New(ptr)
end

-- Methods
function CharacterController:Teleport(pos) -- TP to a position (vec3)
    CC_TP_Internal(self, pos)
end

function CharacterController:TeleportToTarget(obj) -- TP to another gameObject
    CC_TPToTarget_Internal(self, obj)
end

function CharacterController:Move(moveDir)
    CC_Move_Internal(self, moveDir)
end

function CharacterController:Jump()
    CC_Jump_Internal(self)
end

function CharacterController:GetVelocity()
    return CC_GetVelocity_Internal(self)
end

function CharacterController:GetVerticalVelocity()
    return CC_GetVerticalVelocity_Internal(self)
end

function CharacterController:IsGrounded()
    return CC_IsGrounded_Internal(self)
end

function CharacterController:IsInWater()
    return CC_IsInWater_Internal(self)
end

function CharacterController:SetVelocityX(x)
    CC_SetVelocityX_Internal(self, x)
end

function CharacterController:SetVelocityY(y)
    CC_SetVelocityY_Internal(self, y)
end

function CharacterController:SetVelocityZ(z)
    CC_SetVelocityZ_Internal(self, z)
end

function CharacterController:SetVelocity(velocity)
    CC_SetVelocity_Internal(self, velocity)
end

return CharacterController