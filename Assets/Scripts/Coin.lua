local Object = require("Object")
local Animator = require("Animator")

local Coin = Object:New()

function Coin:New()
    local instance = { anim = nil }
    setmetatable(instance, { __index = self })
    return instance
end

function Coin:Start()
    self.anim = Animator.Get(self)
--    self:AddTag("Coin")
    self.anim:Play("Qishilong_SC01C11_B_CG3_DH", true)
end


return Coin