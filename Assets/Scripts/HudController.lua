local GameHUD = {}
GameHUD.__index = GameHUD
 
function GameHUD:New(self)
    local instance = setmetatable({}, GameHUD)
    instance.hud    = 0
    instance.hp     = 1.0
    instance.score  = 0
    instance.paused = false
    return instance
end
 
function GameHUD:Start()
    self.hud = CreateHUD("GameHUD")
    if self.hud == 0 then return end
 
    ShowHUD(self.hud)
 
    HUD_SetText(self.hud,         "ScoreLabel", "Score: 0")
    HUD_SetProgress(self.hud,     "HpBar",       1.0)
    HUD_SetBarFillColor(self.hud, "HpBar",       0.2, 0.8, 0.2, 1.0)
    HUD_SetBarBgColor(self.hud,   "HpBar",       0.15, 0.15, 0.15, 1.0)
    HUD_SetVisible(self.hud,      "PausePanel",  false)
    HUD_SetVisible(self.hud,      "DeadPanel",   false)
end
 
function GameHUD:Update(dt)
    if self.paused then return end
    if self.hud == 0 then return end
 
    self.hp = self.hp - dt * 0.05
    if self.hp < 0 then self.hp = 0 end
 
    HUD_SetProgress(self.hud, "HpBar", self.hp)
 
    if self.hp > 0.6 then
        HUD_SetBarFillColor(self.hud, "HpBar", 0.2, 0.8, 0.2, 1.0)
    elseif self.hp > 0.3 then
        HUD_SetBarFillColor(self.hud, "HpBar", 0.95, 0.6, 0.05, 1.0)
    else
        HUD_SetBarFillColor(self.hud, "HpBar", 0.9, 0.15, 0.1, 1.0)
    end
 
    if self.hp <= 0 then
        HUD_SetVisible(self.hud, "DeadPanel", true)
        HUD_SetText(self.hud,    "DeadLabel", "YOU DIED")
    end
end
 
function GameHUD:TakeDamage(amount)
    if self.hud == 0 then return end
    self.hp = self.hp - amount
    if self.hp < 0 then self.hp = 0 end
    HUD_SetProgress(self.hud, "HpBar", self.hp)
end
 
function GameHUD:Heal(amount)
    if self.hud == 0 then return end
    self.hp = self.hp + amount
    if self.hp > 1.0 then self.hp = 1.0 end
    HUD_SetProgress(self.hud, "HpBar", self.hp)
end
 
function GameHUD:AddScore(points)
    if self.hud == 0 then return end
    self.score = self.score + points
    HUD_SetText(self.hud, "ScoreLabel", "Score: " .. tostring(self.score))
end
 
function GameHUD:SetPaused(p)
    if self.hud == 0 then return end
    self.paused = p
    HUD_SetVisible(self.hud, "PausePanel", p)
end
 
return GameHUD