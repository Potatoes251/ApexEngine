local AIController = require("AIController")
local ViewComp = require("ViewComponent")
local Collider = require("Collider")
local Object = require("Object")
local Input = require("Input")
local Vector3 = require("Vector3")

local Bowser = Object:New()

function Bowser:New()
    local instance = { 
     -- component
    aic = nil, 
    view = nil,
    -- variables
    hp = 3,
    }
    setmetatable(instance, { __index = self })
    return instance
end

function Bowser:Start()
    self.aic = AIController.Get(self)
    self.view = ViewComp.Get(self)
    self:AddTag("Bowser")
end

function Bowser:Update()
    local obj = self.view:GetClosestWithTag("Player")
    self.aic:SetTarget(obj)
end

function Bowser:TakeDamage()
    hp = hp - 1
    if hp <= 0 then
        self:Destroy()
    end
end

return Bowser