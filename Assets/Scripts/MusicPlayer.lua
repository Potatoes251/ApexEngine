local Object = require("Object")
local AudioComponent = require("AudioComp")

local MusicPlayer = Object:New()

function MusicPlayer:New()
    local instance = {
    audio = nil,
    }
    setmetatable(instance, { __index = self })
    return instance
end

function MusicPlayer:Start()
    self.audio = AudioComponent.Get(self)
    self.audio:Play()
end


return MusicPlayer