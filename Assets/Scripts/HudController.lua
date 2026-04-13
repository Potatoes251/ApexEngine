-- HudController.lua
-- Attach this ScriptComponent to any object in the scene to control HUDs.
--
-- Available functions (registered globally by the engine):
--   handle = CreateHUD("Name")    load Assets/***/Name.hud, returns integer handle
--   ShowHUD(handle)               make that HUD visible
--   HideHUD(handle)               hide it (keeps the instance alive)
--   DestroyHUD(handle)            free the instance
--   visible = IsHUDVisible(handle)
 
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
    -- The name must match the .hud filename stem, e.g. "Assets/HUDs/GameHUD.hud"
    self.gameHud  = CreateHUD("GameHUD")
    self.pauseHud = CreateHUD("PauseHUD")
 
    -- Show the game HUD immediately; keep pause HUD hidden
    if self.gameHud  ~= 0 then ShowHUD(self.gameHud)  end
    if self.pauseHud ~= 0 then HideHUD(self.pauseHud) end
end
 
function HudController:Update(dt)
    -- Toggle pause HUD on Escape (example — wire to actual input)
    -- self.paused = not self.paused
    -- if self.paused then
    --     HideHUD(self.gameHud)
    --     ShowHUD(self.pauseHud)
    -- else
    --     ShowHUD(self.gameHud)
    --     HideHUD(self.pauseHud)
    -- end
end
 
return HudController