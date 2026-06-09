local AIController = require("AIController")
local ViewComp = require("ViewComponent")
local Animator = require("Animator")
local Collider = require("Collider")
local Object = require("Object")
local Vector3 = require("Vector3")

local Bowser = Object:New()

function Bowser:New()
    local instance = { 
     -- component
    aic = nil, 
    view = nil,
    anim = nil,
    -- variables
    hp = 3,
    }
    setmetatable(instance, { __index = self })
    return instance
end

function Bowser:Start()
    self.aic = AIController.Get(self)
    self.view = ViewComp.Get(self)
    self.anim = Animator.Get(self)
    self:AddTag("Bowser")
    self.anim:Play("Idle", true)
end

function Bowser:Update()
    local obj = self.view:GetClosestWithTag("Player")
    self.aic:SetTarget(obj)
end

function Bowser:TakeDamage()
    self.hp = self.hp - 1
    if self.hp <= 0 then
        self:Destroy()
    end
end

function Bowser:OnTriggerEnter(col)
    local obj = col:GetOwner()
    if obj:HasTag("Projectile") then
        self:TakeDamage()
    end
end

return Bowser