local Collider = require("Collider")
local Object = require("Object")
local Input = require("Input")
local Vector3 = require("Vector3")

local BossFighter = Object:New()

function BossFighter:New()
    local instance = { 
    -- component
    -- variables
    boss = nil,
    player = nil,
    canAttack = false,
    attackTimer = 0,
    }
    setmetatable(instance, { __index = self })
    return instance
end

function BossFighter:Start()
    self.boss = Cast("Bowser", "Bowser")
    self.player = Cast("Player", "Player")
end

function BossFighter:Update(dt)
    if not self.canAttack then return end

    self.attackTimer = self.attackTimer + dt

    if Input.IsKeyDown("R") and self.attackTimer > 1 then
        self:Attack()
        self.attackTimer = 0
        Log("Attack")
    end
end

function BossFighter:OnTriggerEnter(col)
    local obj = col:GetOwner()
    if obj:HasTag("Tail") then
        self.canAttack = true
    elseif obj:HasTag("Bowser") then
        self.player:TakeDamage()
    end
end

function BossFighter:OnTriggerExit(col)
    local obj = col:GetOwner()
    if obj:HasTag("Tail") then
        self.canAttack = false
    end
end

function BossFighter:Attack()
    self.boss:TakeDamage()
end

return BossFighter