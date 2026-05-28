local Object = require("Object")
 
local HealPickup = Object:New()
 
function HealPickup:New(self)
    local inst = {}
    setmetatable(inst, { __index = HealPickup })
    inst.HealAmount  = 1         -- segments restored

    inst.__exposed = {
        "HealAmount"
    }
    return inst
end
 
function HealPickup:Start()
    self:AddTag("HealPickup")
end
 
function HealPickup:Update(dt)
end

function HealPickup:OnTriggerEnter(col)
    local obj = col:GetOwner()
    if not obj:HasTag("Player") then return end

    local player = Cast(obj:GetName(), "Player")

    if player == nil then return end
    player:Heal(self.HealAmount)
    self:Destroy()
end
 
-- OnTriggerEnter handled on the Player side via tag "HealPickup"
 
return HealPickup
