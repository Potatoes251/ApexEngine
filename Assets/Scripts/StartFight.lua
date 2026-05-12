local Object = require("Object")

local Fight = Object:New()

function Fight:New()
    local instance = {}
    setmetatable(instance, { __index = self })
    return instance
end

function Fight:OnTriggerEnter(col)
    -- start the battle
end

return Fight