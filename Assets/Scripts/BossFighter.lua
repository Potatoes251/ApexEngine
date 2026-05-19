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
    }
    setmetatable(instance, { __index = self })
    return instance
end

function BossFighter:Start()
    self.boss = Cast("Bowser", "Bowser")
end

function BossFighter:Update(dt)

end

function BossFighter:OnTriggerEnter(col)
    local obj = col:GetOwner()
    Log("vqfjknefjnfveqnvfrnivfsifvsdnjivdfs")
    if obj:HasTag("Tail") then
        self:Attack()
    end
end

function BossFighter:Attack()
    self.boss:TakeDamage()
end

return BossFighter