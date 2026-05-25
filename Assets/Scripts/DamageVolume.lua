local Object = require("Object")
 
local DamageVolume = Object:New()
 
function DamageVolume:New(self)
    local inst = {}
    setmetatable(inst, { __index = DamageVolume })
    inst.Damage         = 1      -- segments removed per hit
    inst.IsInstant      = false  -- true = destroy self after first hit (spike, etc.)

    inst.__exposed = {
        "Damage",
        "IsInstant"
    }
    return inst
end
 
function DamageVolume:Start()
    self:AddTag("Hazard")
end
 
function DamageVolume:Update(dt)
    
end
 
function DamageVolume:OnTriggerEnter(col)
    local obj = col:GetOwner()
    if not obj:HasTag("Player") then return end
    
    local player = Cast(obj:GetName(), "Player")
    if player == nil then return end
 
    player:TakeDamage(self.Damage)
 
    if self.IsInstant then self:Destroy() end
end
 
return DamageVolume
