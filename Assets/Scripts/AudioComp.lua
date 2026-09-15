local Component = require("Component")

local AudioComponent = {}
AudioComponent.__index = AudioComponent
setmetatable(AudioComponent, { __index = Component })

-- Constructor
function AudioComponent:New(ptr)
    local instance = Component.New(self, ptr)
    return instance
end

-- Public function: fetch controller for an object
function AudioComponent.Get(obj)
    local ptr = GetAudioComp_Internal(obj)  -- C++ function returns pointer
    if not ptr then
        return nil
    end
    return AudioComponent:New(ptr)
end

-- Methods
function AudioComponent:Play()
    AudioComp_Play_Internal(self)
end

-- play 3D sound at object position
function AudioComponent:Play3D()
    AudioComp_Play3D_Internal(self)
end

-- play 3D sound at given position
function AudioComponent:PlayAt(pos)
    return AudioComp_PlayAt_Internal(self, pos)
end

function AudioComponent:Stop()
    AudioComp_Stop_Internal(self)
end

function AudioComponent:SetVolume(vol)
    AudioComp_SetVolume_Internal(self, vol)
end

function AudioComponent:SetSound(soundName)
    AudioComp_SetSound_Internal(self, soundName)
end

function AudioComponent:SetChannel(channel)
    AudioComp_SetChannel_Internal(self, channel)
end

function AudioComponent:SetLoop(loop)
    AudioComp_SetLoop_Internal(self, loop)
end

return AudioComponent