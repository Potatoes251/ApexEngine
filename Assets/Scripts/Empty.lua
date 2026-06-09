local Object = require("Object")
local Camera = require("Camera")

local Empty = Object:New()

function Empty:New()
    local instance = { camera = nil }
    setmetatable(instance, { __index = self })
    return instance
end

function Empty:Start()
    self:SetEnabled(false)
    self.camera = Camera.Get(self)
    self.camera:SetEnabled(false)
    self.camera:SetYaw(170)
    self.camera:SetPitch(-25)
end

function Empty:GetCam()
    return self.camera
end

return Empty