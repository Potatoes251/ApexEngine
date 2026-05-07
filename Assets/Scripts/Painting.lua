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
    loadTime = 1,
    startEffect = false,
    targetScene = "",
    }

    instance.__exposed = {
        "targetScene",
        "loadTime",
    }

    setmetatable(instance, { __index = self })
    return instance
end

function Painting:Start()
    self.mesh = Mesh.Get(self)
end

function Painting:Update(dt)
    if self.startEffect then
        self.accumulatedTime = self.accumulatedTime + dt
        self.mesh:SetUniform("uTime", self.accumulatedTime)
        if self.accumulatedTime > loadTime then
            LoadScene(self.targetScene)
        end

    end
end

function Painting:OnTriggerEnter(col)
    local obj = col:GetOwner()
    if not self.startEffect and obj:HasTag("Player") then
        self.startEffect = true
    end
end


return Painting