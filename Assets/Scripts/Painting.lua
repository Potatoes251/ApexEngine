local Collider = require("Collider")
local Object = require("Object")
local Mesh = require("Mesh")

local Painting = Object:New()


function Painting:New()
    local instance = { 
     -- component
    mesh = nil,
     -- variables
    accumulatedTime = 0,
    }
    setmetatable(instance, { __index = self })
    return instance
end

function Painting:Start()
    self.mesh = Mesh.Get(self)
end

function Painting:Update(dt)
    self.accumulatedTime = self.accumulatedTime + dt
    self.mesh:SetUniform("uTime", self.accumulatedTime)
end




return Painting