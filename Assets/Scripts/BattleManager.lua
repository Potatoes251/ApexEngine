local CharacterController = require("CharacterController")
local Object = require("Object")
local Vector3 = require("Vector3")
local Camera = require("Camera")

local Fight = Object:New()

function Fight:New()
    local instance = {
    hud = nil,
    
    enemy = nil,
    player = nil,
    camera = nil,
    battleCam = nil,

    playerScript = nil,
    playerTargetPos = nil,
    oldPos = nil,

    playerTurn = true,
    battleActive = false,
    enemyAttackTimer = 0,

    isInGrass = false,
    encounterTimer = 0,
    encounterChance = 10, -- 1 in encounterChance
    }
    setmetatable(instance, { __index = self })
    return instance
end

function Fight:Start()
    self.hud = CreateHUD("Fight")

    self.camera = Camera.Get(self)

    self.enemy = Cast("Enemy", "Fighter")
    self.player = Cast("Player", "Fighter")
    self.playerScript = Cast("Player", "Player")
    self.playerTargetPos = Cast("playerArenaPos", "Empty")

    self.enemy:SetOpponent(self.player)
    self.player:SetOpponent(self.enemy)
    self.enemy:SetHuD(self.hud, "EnemyLife")
    self.player:SetHuD(self.hud, "PlayerLife")
    self.enemy:SetBattleMng(self)
    self.player:SetBattleMng(self)
end

function Fight:CheckEncounter(dt)
    if not isInGrass then return end

    self.encounterTimer = self.encounterTimer + dt
    if self.encounterTimer > 1 then
        if math.random(self.encounterChance) == 1 then
            self:OnBattleStart()
        end
        self.encounterTimer = 0
    end
end

function Fight:Update(dt)
    if not self.battleCam then
        self.battleCam = self.playerTargetPos:GetCam()
    end

    if not self.battleActive then 
        self:CheckEncounter(dt)
        return
    end

    self.enemyAttackTimer = self.enemyAttackTimer + dt

    if not self.playerTurn and self.enemyAttackTimer > 1.0 then
        self.enemyAttackTimer = 0
        self.enemy:Attack(1)
    end
end

function Fight:OnTriggerEnter(col)
    isInGrass = true
end

function Fight:OnTriggerExit(col)
    isInGrass = false
end

function Fight:OnBattleStart()
    if self.battleActive then return end

    ShowMouse()
    self.camera:SetEnabled(false)
    self.battleCam:SetMain()

    self.battleActive = true
    self.oldPos = self.playerScript:GetPosition()
    if self.hud ~= 0 then 
        ShowHUD(self.hud)
    end
    self.playerScript.cc:TeleportToTarget(self.playerTargetPos)
    self.playerScript:SetEnabled(false)
    self.playerScript:StopAnim()

    self.player:OnBattleStart()
    self.enemy:OnBattleStart()

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
    self.playerScript.cc:Teleport(self.oldPos)
    self.playerScript:SetEnabled(true)
    self.playerScript:StartAnim()
    HideMouse()
    self.camera:SetEnabled(true)
    self.camera:SetMain()
end

function Fight:StartPlayerTurn()
    self.playerTurn = true
    self.player.isMyTurn = true
    self.enemy.isMyTurn = false
end

function Fight:StartEnemyTurn()
    self.playerTurn = false
    self.player.isMyTurn = false
    self.enemy.isMyTurn = true
    self.enemyAttackTimer = 0
end

function Fight:OnTurnEnd(fighter)
    if fighter == self.player then
        self:StartEnemyTurn()
    else
        self:StartPlayerTurn()
    end
end



return Fight