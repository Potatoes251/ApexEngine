local MenuHUD = {}
MenuHUD.__index = MenuHUD

function MenuHUD:New(self)
    local instance = setmetatable({}, MenuHUD)
    return instance
end

function MenuHUD:Resume()
    if self.hud == 0 then return end
    HideHUD(self.hud)
end

function MenuHUD:Quit()
    if self.hud == 0 then return end
    --TODO: add function to quit the game
end

function MenuHUD:GetToSettingMenu( ... )
	if self.hud == 0 then return end
    HideHUD(self.hud)
    --TODO: show 'Setting' HUD
end

function MainMenuHUD:Start()
    self.hud = CreateHUD("MainMenuHUD")
    if self.hud == 0 then return end

    HUD_SetButtonCallBack(self.hud, "TODO", Resume)
    HUD_SetButtonCallBack(self.hud, "TODO", Quit)
    HUD_SetButtonCallBack(self.hud, "TODO", GetToSettingMenu)
end