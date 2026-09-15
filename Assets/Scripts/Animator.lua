local Component = require("Component")

local Animator = {}
Animator.__index = Animator
setmetatable(Animator, { __index = Component })

-- Constructor
function Animator:New(ptr)
    local instance = Component.New(self, ptr)
    return instance
end

-- Public function: fetch controller for an object
function Animator.Get(obj)
    local ptr = GetAnimator_Internal(obj)  -- C++ function returns pointer
    if not ptr then
        return nil
    end
    return Animator:New(ptr)
end

-- Methods
function Animator:Play(name, interpole)
    return Anim_Play_Internal(self, name, interpole)
end

function Animator:StartBlend(animA, animB, ratio, interpole)
    return Anim_Blend_Internal(self, animA, animB, ratio, interpole)
end

function Animator:SetBlendRatio(ratio)
    return Anim_SetBlendRatio_Internal(self, ratio)
end

function Animator:StartCrossfade(animA, animB, crossfadeTime, interpole)
    return Anim_Crossfade_Internal(self, animA, animB, crossfadeTime, interpole)
end

return Animator