-- done in a state in which I needed to use the git branch 'menu' but I need to have the branch 'HUD' merged in the 'main' and then merge 'main' and 'menu'

local MainMenuHUD = {}
MainMenuHUD.__index = MainMenuHUD

function MainMenuHUD:New(self)
    local instance = setmetatable({}, MainMenuHUD)
    return instance
end

function MainMenuHUD:Resume()
    if self.hud == 0 then return end
    HideHUD(self.hud)
end

function MainMenuHUD:Quit()
    if self.hud == 0 then return end
    --TODO: add function to quit the game
end

function MainMenuHUD:LoadGame()
    if self.hud == 0 then return end
    --TODO: add function to load game files
end

function MainMenuHUD:GetToSettingMenu( ... )
	if self.hud == 0 then return end
    HideHUD(self.hud)
    --TODO: show 'Setting' HUD
end

function MainMenuHUD:Start()
    self.hud = CreateHUD("MainMenuHUD")
    if self.hud == 0 then return end

    HUD_SetButtonCallBack(self.hud, "TODO", Resume)
    HUD_SetButtonCallBack(self.hud, "TODO", Quit)
    HUD_SetButtonCallBack(self.hud, "TODO", LoadGame)
    HUD_SetButtonCallBack(self.hud, "TODO", GetToSettingMenu)
end
