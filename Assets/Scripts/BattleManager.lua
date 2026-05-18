local CharacterController = require("CharacterController")
local Object = require("Object")

local Fight = Object:New()

function Fight:New()
    local instance = {
    cc = nil,
    hud = nil,
    enemy = nil,
    player = nil,
    playerTargetPos = nil,
    }
    setmetatable(instance, { __index = self })
    return instance
end

function Fight:Start()
    self.hud = CreateHUD("Fight")
    self.cc = CharacterController.Get(self)
    self.enemy = Cast("Enemy", "Fighter")
    self.player = Cast("Player", "Fighter")
    self.playerTargetPos = Cast("playerArenaPos")

    self.enemy:SetOpponent(self.player)
    self.player:SetOpponent(self.enemy)
end

function Fight:OnTriggerEnter(col)
    if self.hud ~= 0 then 
        ShowHUD(self.hud) 
    end
    self.cc:TeleportToTarget(self.playerTargetPos)
end

return Fight