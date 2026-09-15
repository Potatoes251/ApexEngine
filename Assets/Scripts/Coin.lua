local Object = require("Object")

local Coin = Object:New()

function Coin:New()
    local instance = {}
    setmetatable(instance, { __index = self })
    return instance
end

function Coin:Start()
    self:AddTag("Coin")
end

return Coin