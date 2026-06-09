local Collider = require("Collider")
local Object = require("Object")
local Mesh = require("Mesh")
local AudioComponent = require("AudioComp")

local Painting = Object:New()


function Painting:New()
    local instance = { 
     -- component
    mesh = nil,
     -- variables
    accumulatedTime = 0,
    LoadTime = 1,
    startEffect = false,
    TargetScene = "",
    }

    instance.__exposed = {
        "TargetScene",
        "LoadTime",
    }

    setmetatable(instance, { __index = self })
    return instance
end

function Painting:Start()
    self.mesh = Mesh.Get(self)
    self.mesh:SetUniform("uTime", 0)
    self.accumulatedTime = 0
end

function Painting:Update(dt)
    if self.startEffect then
        self.accumulatedTime = self.accumulatedTime + dt
        self.mesh:SetUniform("uTime", self.accumulatedTime)
        if self.accumulatedTime > self.LoadTime then
            LoadScene(self.TargetScene)
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