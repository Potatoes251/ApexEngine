local AIController = require("AIController")
local ViewComp = require("ViewComponent")
local Collider = require("Collider")
local Object = require("Object")
local Input = require("Input")
local Vector3 = require("Vector3")

local Goomba = Object:New()

function Goomba:New()
    local instance = { 
     -- component
    aic = nil, 
    view = nil,
    -- variables
    }
    setmetatable(instance, { __index = self })
    return instance
end

function Goomba:Start()
    self.aic = AIController.Get(self)
    self.view = ViewComp.Get(self)
    self:AddTag("Goomba")
end

function Goomba:Update()
    local obj = self.view:GetClosestWithTag("Player")
    self.aic:SetTarget(obj)
end

return Goomba