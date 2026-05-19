local Object = require("Object")
local Input = require("Input")

local Fighter = Object:New()

function Fighter:New()
    local instance = { 
    -- component
    opponent = nil,
    -- variables
    battleManager = nil,
    hud = nil,
    barName = "",
    hp = 0,
    maxhp = 10,
    isMyTurn = false,
    }
    setmetatable(instance, { __index = self })
    return instance
end

function Fighter:SetOpponent(opp)
    self.opponent = opp
end

function Fighter:SetBattleMng(bm)
    self.battleManager = bm
end

function Fighter:SetHuD(hud, barName)
    self.hud = hud
    self.barName = barName
end

function Fighter:OnBattleStart()
    self.hp = self.maxhp
    HUD_SetProgress(self.hud, self.barName, 1.0)
end

function Fighter:Start()
    self:SetEnabled(false)
end

function Fighter:Update(dt)
    if self.isMyTurn  then
        if Input.IsKeyDown("P") then
            self:Attack()
        elseif Input.IsKeyDown("O") then
            self:Flee()
        end
    end
end

function Fighter:TakeDamage(damage)
    self.hp = self.hp - damage

    if self.hp <= 0 then
        self.battleManager:OnBattleEnd()
    end

    HUD_SetProgress(self.hud, self.barName, self.hp / self.maxhp)
end

function Fighter:Attack()
    if self.hp <= 0 then return end

    self.isMyTurn = false
    if self.opponent ~= nil then
        self.opponent:TakeDamage(1)
    end
    if self.battleManager ~= nil then
        self.battleManager:OnTurnEnd(self)
    end
end

function Fighter:Flee()
    if self.hp <= 0 then return end
    
    if self.battleManager ~= nil then
        self.battleManager:OnBattleEnd()
    end
end


return Fighter