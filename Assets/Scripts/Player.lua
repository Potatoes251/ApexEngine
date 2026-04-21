local CharacterController = require("CharacterController")
local Animator = require("Animator")
local Collider = require("Collider")
local Camera = require("Camera")
local Object = require("Object")
local Input = require("Input")

local Player = Object:New()

function Player:New()
    local instance = { cc = nil, cam = nil, anim = nil, coinCount = 0 }
    setmetatable(instance, { __index = self })
    return instance
end

function Player:Start()
    self.cc = CharacterController.Get(self)
    self.anim = Animator.Get(self)
    self.cam = Camera.Get(self)
    self:AddTag("Player")
    self.anim:StartBlend("Idle", "Walking", 0, true)
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

    local speed = self.cc:GetVelocity()
    local walkSpeed = 2.0
    local blend = speed / walkSpeed

    -- clamp between 0 and 1
    if blend > 1 then blend = 1 end

    self.anim:SetBlendRatio(blend)

    if (Input.IsKeyDown("Space")) then
        self.cc:Jump()
    end
end

function Player:OnTriggerEnter(col)
    local obj = col:GetOwner()
    if (obj:HasTag("Coin")) then
        self.coinCount = self.coinCount + 1
        obj:Destroy()
    end
end

return Player