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
    playerTurn = true,
    battleActive = false,
    enemyAttackTimer = 0,
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
    self.enemy:SetBattleMng(self)
    self.player:SetBattleMng(self)
end

function Fight:Update(dt)
    if not self.battleActive then return end

    self.enemyAttackTimer = self.enemyAttackTimer + dt

    if not self.playerTurn and self.enemyAttackTimer > 1.0 then
        self.enemyAttackTimer = 0
        self.enemy:Attack(1)
    end
end

function Fight:OnTriggerEnter(col)
    self:OnBattleStart()
end

function Fight:OnBattleStart()
    self.battleActive = true
    if self.hud ~= 0 then 
        ShowHUD(self.hud)
    end
    self.cc:TeleportToTarget(self.playerTargetPos)
    self.playerScript:SetEnabled(false)
    self.playerScript:StopAnim()
    self.player:SetEnabled(true)

    if math.random(0, 1) == 0 then
        self:StartPlayerTurn()
    else
        self:StartEnemyTurn()
    end
end

function Fight:OnBattleEnd()
    self.battleActive = false
    if self.hud ~= 0 then 
        HideHUD(self.hud)
    end
    -- self.cc:TeleportToTarget(self.playerTargetPos) -- TODO: reset to old pos
    self.playerScript:SetEnabled(true)
    self.player:SetEnabled(false)
end

function Fight:StartPlayerTurn()
    self.playerTurn = true
    self.player.isMyTurn = true
    self.enemy.isMyTurn = false

    Log("Player's turn")
end

function Fight:StartEnemyTurn()
    self.playerTurn = false
    self.player.isMyTurn = false
    self.enemy.isMyTurn = true
    self.enemyAttackTimer = 0
    Log("Enemy's turn")
end

function Fight:OnTurnEnd(fighter)
    if fighter == self.player then
        self:StartEnemyTurn()
    else
        self:StartPlayerTurn()
    end
end



return Fight