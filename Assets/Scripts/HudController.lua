local HudController = {}
HudController.__index = HudController
 
local MAX_SEGMENTS = 8
local MAX_LIVES    = 3
 
-- Colours per health state
local COLOR_FULL    = { 0.20, 0.85, 0.25, 1.0 }   -- green
local COLOR_MID     = { 0.95, 0.60, 0.05, 1.0 }   -- orange  (≤4 segments)
local COLOR_LOW     = { 0.90, 0.15, 0.10, 1.0 }   -- red     (≤2 segments)
local COLOR_EMPTY   = { 0.18, 0.18, 0.18, 1.0 }   -- dark grey (empty slot)
 
function HudController:New(self)
    local inst = setmetatable({}, HudController)
    inst.hud      = 0
    inst.score    = 0
    inst.segments = MAX_SEGMENTS   -- current filled segments (0-8)
    inst.lives    = MAX_LIVES
    inst.paused   = false
    return inst
end
 
function HudController:Start()
    self.hud = CreateHUD("GameHUD")
    if self.hud == 0 then return end
    ShowHUD(self.hud)
 
    self:RefreshSegments()
    self:RefreshLives()
    HUD_SetText(self.hud, "ScoreLabel", "Score: 0")
    HUD_SetVisible(self.hud, "PausePanel",    false)
    HUD_SetVisible(self.hud, "DeadPanel",     false)
    HUD_SetVisible(self.hud, "GameOverPanel", false)
end
 
function HudController:Update(dt)
end
 
function HudController:RefreshSegments()
    if self.hud == 0 then return end
    for i = 1, MAX_SEGMENTS do
        local name  = "HpSeg" .. i
        local filled = (i <= self.segments)
 
        -- Each segment is either filled (coloured) or empty (dark)
        HUD_SetProgress(self.hud, name, filled and 1.0 or 0.0)
 
        if filled then
            if self.segments <= 2 then
                HUD_SetBarFillColor(self.hud, name,
                    COLOR_LOW[1], COLOR_LOW[2], COLOR_LOW[3], COLOR_LOW[4])
            elseif self.segments <= 4 then
                HUD_SetBarFillColor(self.hud, name,
                    COLOR_MID[1], COLOR_MID[2], COLOR_MID[3], COLOR_MID[4])
            else
                HUD_SetBarFillColor(self.hud, name,
                    COLOR_FULL[1], COLOR_FULL[2], COLOR_FULL[3], COLOR_FULL[4])
            end
        else
            HUD_SetBarFillColor(self.hud, name,
                COLOR_EMPTY[1], COLOR_EMPTY[2], COLOR_EMPTY[3], COLOR_EMPTY[4])
        end
    end
end
 
function HudController:RefreshLives()
    if self.hud == 0 then return end
    HUD_SetText(self.hud, "LivesLabel", "x" .. tostring(self.lives))
end
 
-- Removes one segment. Returns true if the player just died (segments == 0).
function HudController:TakeDamage(amount)
    if self.hud == 0 then return false end
    amount = amount or 1
    self.segments = math.max(0, self.segments - math.floor(amount))
    self:RefreshSegments()
 
    if self.segments <= 0 then
        self:OnHpDepleted()
        return true
    end
    return false
end
 
function HudController:Heal(amount)
    if self.hud == 0 then return end
    amount = amount or 1
    self.segments = math.min(MAX_SEGMENTS, self.segments + math.floor(amount))
    self:RefreshSegments()
end
 
function HudController:RestoreFullHP()
    self.segments = MAX_SEGMENTS
    self:RefreshSegments()
end
 
function HudController:GetSegments()   return self.segments end
function HudController:GetLives()      return self.lives    end
 
function HudController:AddScore(points)
    if self.hud == 0 then return end
    self.score = self.score + points
    HUD_SetText(self.hud, "ScoreLabel", "Score: " .. tostring(self.score))
end
 
function HudController:SetPaused(p)
    if self.hud == 0 then return end
    self.paused = p
    HUD_SetVisible(self.hud, "PausePanel", p)
end
 
function HudController:OnHpDepleted()
    self.lives = self.lives - 1
    self:RefreshLives()
 
    if self.lives <= 0 then
        HUD_SetVisible(self.hud, "GameOverPanel", true)
    else
        -- Restore HP for the next life — player script handles respawn
        self.segments = MAX_SEGMENTS
        self:RefreshSegments()
    end
end
 
return HudController