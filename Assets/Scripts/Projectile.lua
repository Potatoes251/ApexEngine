local Component = require("Component")

local Projectile = {}
Projectile.__index = Projectile
setmetatable(Projectile, { __index = Component })

function Projectile:New(ptr)
    local instance = Component.New(self, ptr)
    return instance
end

function Projectile.Get(obj)
    local ptr = GetProjectileComponent_Internal(obj.__object)  
    if not ptr then
        return nil
    end
    return Projectile:New(ptr)
end

function Projectile:Setup(directionVec3, speed, lifeSpan)
    ProjectileSetup_Internal(self, directionVec3, speed, lifeSpan)
end

return Projectile
