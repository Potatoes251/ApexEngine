local CharacterController = require("CharacterController")
local Camera = require("Camera")
local Object = require("Object")
local Input = require("Input")

local Player = Object:New()

function Player:New()
    local instance = { cc = nil, cam = nil }
    setmetatable(instance, { __index = self })
    return instance
end

function Player:Start()
    self.cc = CharacterController.Get(self)
    self.cam = Camera.Get(self)
end

function Player:Update(dt)
    local x = Input.GetAxis("Horizontal")
    local z = Input.GetAxis("Vertical")

    if x ~= 0 or z ~= 0 then
        local forward = self.cam:GetFront()
        local right   = self.cam:GetRight()
        forward.y = 0
        right.y = 0

        forward = forward:Normalized()
        right   = right:Normalized()
        local moveDir = (right * x) + (forward * z)

        self.cc:Move(moveDir)
    end

    if (Input.IsKeyDown("Space")) then
        self.cc:Jump()
    end
end

return Player