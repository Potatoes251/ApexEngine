local Object = require("Object")

local Fight = Object:New()

function Fight:New()
    local instance = {}
    setmetatable(instance, { __index = self })
    return instance
end

function Fight:Start()
    self.hud = CreateHUD("Fight")
end

function Fight:OnTriggerEnter(col)
    if self.hud == 0 then return end
    ShowHUD(self.hud)
end

return Fight