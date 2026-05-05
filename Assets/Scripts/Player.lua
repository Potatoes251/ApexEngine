local CharacterController = require("CharacterController")
local Animator = require("Animator")
local Collider = require("Collider")
local Camera = require("Camera")
local Object = require("Object")
local Input = require("Input")

local Player = Object:New()

function Player:New()
    local instance = { 
     -- component
    cc = nil, 
    cam = nil, 
    anim = nil,
    -- variables
    coinCount = 0;
    wasGrounded = false, 
    wasInWater = false, 
    accumulatedTime = 0,
    smoothBlend = 0,
    }
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

    local speed     = self.cc:GetVelocity()
    local walkSpeed = 2.0
    local targetBlend = math.min(speed / walkSpeed, 1)
    
    self.smoothBlend = self.smoothBlend + (targetBlend - self.smoothBlend) * math.min(5.0 * dt, 1)

    self.anim:SetBlendRatio(self.smoothBlend)

    isGrounded = self.cc:IsGrounded()
    isInWater = self.cc:IsInWater()

    if Input.IsKeyDown("Space") and isGrounded and not isInWater then
        if (blend > 0.5) then
            self.anim:StartCrossfade("Walking", "Jump", 0.5, true)
        else
            self.anim:StartCrossfade("Idle", "Jump", 0.5, true)
        end
        self.cc:Jump()
    end

    if not self.wasGrounded and isGrounded and not isInWater then 
        self.anim:StartBlend("Idle", "Walking", 0, true)
    elseif not self.wasInWater and isInWater then
        self.anim:StartBlend("Idle", "Swimming", 0, true)
    elseif self.wasInWater and not isInWater then
        self.anim:StartBlend("Idle", "Walking", 0, true)
    end

    self.wasInWater = isInWater
    self.wasGrounded = isGrounded
end

function Player:OnTriggerEnter(col)
    local obj = col:GetOwner()
    if (obj:HasTag("Coin")) then
        self.coinCount = self.coinCount + 1
        obj:Destroy()
    end
end

return Player