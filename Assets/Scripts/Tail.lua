local Object = require("Object")

local Tail = Object:New()

function Tail:New()
    local instance = {}
    setmetatable(instance, { __index = self })
    return instance
end

function Tail:Start()
    self:AddTag("Tail")
end


return Tail