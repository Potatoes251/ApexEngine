local CharacterController = require("CharacterController")
local Animator = require("Animator")
local Collider = require("Collider")
local Camera = require("Camera")
local Object = require("Object")
local Input = require("Input")
local Vector3 = require("Vector3")

local Player = Object:New()

function Player:New()
    local instance = { 
     -- component
    cc = nil, 
    cam = nil, 
    anim = nil,
    hud = nil,
    -- variables
    coinCount = 0;
    wasGrounded = false, 
    wasInWater = false, 
    accumulatedTime = 0,
    smoothBlend = 0,
    }
    setmetatable(instance, { __index = Player })
    return instance
end

function Player:Start()
    self.cc = CharacterController.Get(self)
    self.anim = Animator.Get(self)
    self.cam = Camera.Get(self)
    self:AddTag("Player")
    self.anim:StartBlend("Idle", "Walking", 0, true)

    self.hud = Cast("HUDController", "HudController")
    if self.hud == nil then
        print("[Player] WARNING: HudController not found. Make sure an object"
              .. " named 'HUDManager' has HudController.lua attached.")
    end
end

function Player:FixedUpdate(dt)
    self:ProcessInput(dt)

    self.anim:SetBlendRatio(self.smoothBlend)

    isGrounded = self.cc:IsGrounded()
    isInWater = self.cc:IsInWater()

    if Input.IsKeyDown("Space") and isGrounded and not isInWater then
        if self.smoothBlend > 0.5 then
            self.anim:StartCrossfade("Walking", "Jump", 0.5, true)
        else
            self.anim:StartCrossfade("Idle", "Jump", 0.5, true)
        end
        self.cc:Jump()
    end

    if (not self.wasGrounded and isGrounded and not isInWater) or (self.wasInWater and not isInWater) then 
        self.anim:StartBlend("Idle", "Walking", 0, true)
    elseif not self.wasInWater and isInWater then
        self.anim:StartBlend("Idle", "Swimming", 0, true)
    end

    self.wasInWater = isInWater
    self.wasGrounded = isGrounded
end

function Player:ProcessInput(dt)
    local x = Input.GetAxis("Horizontal")
    local z = Input.GetAxis("Vertical")

    local y = 0
    if Input.IsKeyDown("Space") then
        y = 1
    end
    if Input.IsKeyDown("Ctrl") then
        y = y - 1
    end

    local inWater = self.cc:IsInWater()

    if x ~= 0 or z ~= 0 or (y ~= 0 and inWater) then
        local forward = self.cam:GetFront()
        local right   = self.cam:GetRight()
        forward.y = 0
        right.y = 0

        forward = forward:Normalized()
        right   = right:Normalized()
        local moveDir = (right * x) + (forward * z)

        if inWater then
            moveDir = moveDir + Vector3.New(0, y, 0)
        end

        self.cc:Move(moveDir)
    end

    local speed     = self.cc:GetVelocity()
    local walkSpeed = 2.0
    local targetBlend = math.min(speed / walkSpeed, 1)
    
    self.smoothBlend = self.smoothBlend + (targetBlend - self.smoothBlend) * math.min(5.0 * dt, 1)
end

function Player:OnTriggerEnter(col)
    local obj = col:GetOwner()
    if (obj:HasTag("Coin")) then
        self.coinCount = self.coinCount + 1

        if self.hud ~= nil then
            self.hud:AddScore(1)
        end

        obj:Destroy()
    end
end

function Player:GetCoinCount()
    return self.coinCount
end

return Player