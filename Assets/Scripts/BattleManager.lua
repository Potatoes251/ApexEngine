local CharacterController = require("CharacterController")
local Object = require("Object")

local Fight = Object:New()

function Fight:New()
    local instance = {
    cc = nil,
    hud = nil,
    enemy = nil,
    player = nil,
    playerScript = nil,
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
    self.playerScript = Cast("Player", "Player")
    self.playerTargetPos = Cast("playerArenaPos")

    self.enemy:SetOpponent(self.player)
    self.player:SetOpponent(self.enemy)
    self.enemy:SetHuD(self.hud, "EnemyLife")
    self.player:SetHuD(self.hud, "PlayerLife")
end

function Fight:OnTriggerEnter(col)
    self:OnBattleStart()
end

function Fight:OnBattleStart()
    if self.hud ~= 0 then 
        ShowHUD(self.hud) 
    end
    self.cc:TeleportToTarget(self.playerTargetPos)
    self.playerScript:SetEnabled(false)
    self.playerScript:StopAnim()
    self.player:SetEnabled(true)
    self.enemy:SetEnabled(true)
end

return Fight