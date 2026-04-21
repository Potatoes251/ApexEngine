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
function CharacterController:Move(moveDir)
    CC_Move_Internal(self, moveDir)
end

function CharacterController:Jump()
    CC_Jump_Internal(self)
end

function CharacterController:GetVelocity()
    return CC_GetVelocity_Internal(self)
end

return CharacterController