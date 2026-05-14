local Object = require("Object")

local Coin = Object:New()

function Coin:New()
    local instance = { anim = nil }
    setmetatable(instance, { __index = Coin })
    return instance
end

function Coin:Start()
    self:AddTag("Coin")
    if self.anim then
        self.anim:Play("Qishilong_SC01C11_B_CG3_DH", true)
    end
end


return Coin