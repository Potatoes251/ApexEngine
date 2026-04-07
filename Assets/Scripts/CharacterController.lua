local CharacterController = {}
CharacterController.__index = CharacterController

-- Constructor
function CharacterController:New(ptr)
    local instance = { __object = ptr }  -- store the C++ pointer/reference
    setmetatable(instance, { __index = self })
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

return CharacterController