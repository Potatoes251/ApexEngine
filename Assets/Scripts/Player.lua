local CharacterController = require("CharacterController")
local Animator = require("Animator")
local Collider = require("Collider")
local Camera = require("Camera")
local Object = require("Object")
local Input = require("Input")
local Vector3 = require("Vector3")
local SpawnManager = require("SpawnManager")
local Projectile = require("Projectile")
local AudioComponent = require("AudioComp")
local HUDMenu = require("HUDMenu")

local KILL_Z             = -50.0   -- Y below this triggers death
local RESPAWN_INVINCIBLE = 2.5     -- seconds of invincibility after respawn
local DEATH_FREEZE_TIME  = 5.2     -- brief freeze before respawn teleport
local DAMAGE_COOLDOWN    = 0.5     -- min seconds between damage hits

local Player = Object:New()

function Player:New()
    local instance = { 
     -- component
    cc = nil, 
    cam = nil, 
    anim = nil,
    audio = nil,
    hud = nil,
    -- variables
    coinCount = 0,
    wasGrounded = false, 
    wasInWater = false, 
    accumulatedTime = 0,
    smoothBlend = 0,

    spawnX = 0,
    spawnY = 0,
    spawnZ = 0,

    isDead = false,
    isGameOver = false,
    deathTimer = 0,
    invincibleTimer = 0,
    damageTimer = 0,
    canFire = false,
    jumpCombo = 0,
    jumpTimer = 0,
    lungeUpwardForce = 0.2,
    lungeForwardForce = 20,
    groundPoundForce = 25,
    isGroundPounding = false,
    hasLunged = false,
    }
    setmetatable(instance, { __index = self })
    return instance
end

function Player:Start()
    self.cc = CharacterController.Get(self)
    self.anim = Animator.Get(self)
    self.cam = Camera.Get(self)
    self.audio = AudioComponent.Get(self)
    self:AddTag("Player")
    self.anim:StartBlend("Idle", "Walking", 0, true)

    self.hud = Cast("HUDController", "HudController")
    if self.hud == nil then
        Log("HudController not found. Make sure an object named 'HUDController' has HudController.lua attached.")
    end

    local pos = self:GetPosition()
    self.spawnX = pos.x
    self.spawnY = pos.y
    self.spawnZ = pos.z

    self.menu = HUDMenu:New()
    self.menu:Start()
end

function Player:FixedUpdate(dt)
    if self.isGameOver then return end

    if self.isDead then
        self.deathTimer = self.deathTimer - dt
        if self.deathTimer <= 0 then
            self:DoRespawn()
        end
        return
    end

    local pos = self:GetPosition()
    if pos.y < KILL_Z then
        self:TakeDamage(8)  -- insta-kill if we fall off the level
        return
    end

    if self.invincibleTimer > 0 then
        self.invincibleTimer = self.invincibleTimer - dt
    end

    if self.damageTimer > 0 then
        self.damageTimer = self.damageTimer - dt
    end

    self:ProcessInput(dt)

    self.jumpTimer = self.jumpTimer - dt

    self.anim:SetBlendRatio(self.smoothBlend)

    isGrounded = self.cc:IsGrounded()
    isInWater = self.cc:IsInWater()

    --jump
    if (Input.IsKeyDown("Space") and isGrounded and not isInWater and not self.isGroundPounding) then
        if self.smoothBlend > 0.5 then
            self.anim:StartCrossfade("Walking", "Jump", 0.5, true)
        else
            self.anim:StartCrossfade("Idle", "Jump", 0.5, true)
        end

        if self.jumpTimer <= 0 then
            self.jumpCombo = 0
        end

        self.jumpCombo = self.jumpCombo + 1
        self.jumpTimer = 0.5

        if self.jumpCombo == 1 then
            self.cc:SetVelocityY(4)
        elseif self.jumpCombo == 2 then
            self.cc:SetVelocityY(6)
        elseif self.jumpCombo == 3 then
            self.cc:SetVelocityY(8)
        end
    end

    if (not self.wasGrounded and isGrounded and not isInWater) or (self.wasInWater and not isInWater) then 
        self.anim:StartBlend("Idle", "Walking", 0, true)
    elseif not self.wasInWater and isInWater then
        self.anim:StartBlend("Idle", "Swimming", 0, true)
    end

    --lunge
    if (Input.IsKeyDown("E") and not isGrounded and not isInWater and not self.isGroundPounding and not self.hasLunged) then
        local forward = self.cam:GetFront()
        forward.y = 0
        forward = forward:Normalized()
        forward.x = forward.x * self.lungeForwardForce
        forward.z = forward.z * self.lungeForwardForce

        self.hasLunged = true

        self.cc:Move(forward + Vector3.New(0, self.lungeUpwardForce, 0))
    end

    --groundPound
    if (Input.IsKeyDown("Ctrl") and not isGrounded and not isInWater and not self.isGroundPounding) then
        self.isGroundPounding = true
        self.cc:SetVelocityX(0)
        self.cc:SetVelocityY(-self.groundPoundForce)
        self.cc:SetVelocityZ(0)
    end

    if (self.isGroundPounding and isGrounded) then
        self.isGroundPounding = false
    end

    if (self.hasLunged and isGrounded) then
        self.hasLunged = false
    end

    self.wasInWater = isInWater
    self.wasGrounded = isGrounded

    if Input.IsKeyDown("I") then
        self.menu:Open()
    end
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

        self.cc:Move(moveDir:Normalized())
    end

    local speed     = self.cc:GetVelocity()
    local walkSpeed = 2.0
    local targetBlend = math.min(speed / walkSpeed, 1)
    
    self.smoothBlend = self.smoothBlend + (targetBlend - self.smoothBlend) * math.min(5.0 * dt, 1)

    local canFireDown = Input.IsKeyDown("Q")
    if canFireDown and not self.canFire then
        self:ThrowFireball()
    end
    self.canFire = canFireDown
end

function Player:TakeDamage(amount)
    if self.isDead or self.isGameOver then return end
    if self.invincibleTimer > 0       then return end
    if self.damageTimer > 0           then return end
 
    amount = amount or 1
    self.damageTimer = DAMAGE_COOLDOWN
 
    if self.hud ~= nil then
        local died = self.hud:TakeDamage(amount)
        if died then
            if self.hud:GetLives() <= 0 then
                self:GameOver()
            else
                self:Die()
            end
        end
    else
        self:Die()
    end
end
 
function Player:Heal(amount)
    if self.hud ~= nil then self.hud:Heal(amount) end
end

function Player:Die()
    if self.isDead or self.isGameOver then return end
 
    self.isDead     = true
    self.deathTimer = DEATH_FREEZE_TIME
 
    if self.audio ~= nil then
        self.audio:SetSound("Assets/Sounds/marioDeathSound.ogg")
        self.audio:Play()
    end

    if self.hud ~= nil then
        HUD_SetVisible(self.hud.hud, "DeadPanel", true)
    end
end
 
function Player:DoRespawn()
    self.isDead          = false
    self.invincibleTimer = RESPAWN_INVINCIBLE
    self.damageTimer     = 0

    local checkpoint = SpawnManager:GetSpawnPoint()
    local targetSpawn = Vector3.New(self.spawnX, self.spawnY, self.spawnZ)
 
    if checkpoint ~= nil then
        targetSpawn = checkpoint
    end

    -- Teleport back to spawn
    self.cc:Teleport(targetSpawn)
 
    if self.anim then
        self.anim:StartBlend("Idle", "Walking", 0, true)
    end
 
    if self.hud ~= nil then
        HUD_SetVisible(self.hud.hud, "DeadPanel", false)
    end
end
 
function Player:GameOver()
    self.isGameOver = true
    self.isDead     = false
end

function Player:UpdateScore(amount)
    self.coinCount = self.coinCount + amount
    self.hud:AddScore(amount)
end

function Player:OnTriggerEnter(col)
    local obj = col:GetOwner()
    if obj:HasTag("Coin") then
        self.audio:SetSound("Assets/Sounds/Coin.wav")
        self.audio:Play()
        if self.hud ~= nil then
            self:UpdateScore(1)
        end

        obj:DestroyCollectible()
    elseif obj:HasTag("Goomba") then
        if self.cc:GetVerticalVelocity() < 0 then
            obj:Destroy()
        else
            self:TakeDamage(1)
        end
    end
end

function Player:StopAnim()
    self.anim:Play("Idle")
end

function Player:StartAnim()
    self.anim:StartBlend("Idle", "Walking", 0, true)
end

function Player:SetSpawnPoint(x, y, z)
    self.spawnX = x
    self.spawnY = y
    self.spawnZ = z
end

function Player:ThrowFireball()
    local fireball = Object.CreateSphere("Fireball_Projectile")

    self.audio:SetSound("Assets/Sounds/Fireball.wav")
    self.audio:Play()
    
    if fireball then
        fireball:AddTag("Projectile")

        local pos = self:GetPosition() + self:GetForward() * 1.25 + Vector3.New(0, 0.5, 0)
        fireball:SetPosition(pos.x, pos.y, pos.z)
        fireball:SetScale(0.5, 0.5, 0.5)
        
        fireball:AddComponent("RigidBodyComponent") 
        fireball:AddComponent("MeshColliderComponent") 
        fireball:AddComponent("ProjectileComponent")

        local projLogic = Projectile.Get(fireball)
        if projLogic then
            local launchDir = self:GetForward()
            projLogic:Setup(launchDir, 20.0, 3.0)
        end
    end
end
 
function Player:GetCoinCount()  return self.coinCount  end
function Player:IsAlive()       return not self.isDead and not self.isGameOver end
function Player:IsInvincible()  return self.invincibleTimer > 0 end

return Player