
local HudController = {}
HudController.__index = HudController
 
function HudController:New(self)
    local instance = setmetatable({}, HudController)
    instance.gameHud  = 0
    instance.pauseHud = 0
    instance.paused   = false
    return instance
end
 
function HudController:Start()
    -- Load the HUD files once at game start.
    self.gameHud  = CreateHUD("GameHUD")
    self.pauseHud = CreateHUD("PauseHUD")
 
    -- Show the game HUD immediately; keep pause HUD hidden
    if self.gameHud  ~= 0 then ShowHUD(self.gameHud)  end
    if self.pauseHud ~= 0 then HideHUD(self.pauseHud) end
end
 
function HudController:Update(dt)

end
 
return HudController