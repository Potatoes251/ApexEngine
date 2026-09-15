local MenuHUD = {}
MenuHUD.__index = MenuHUD

function MenuHUD:New(self)
    local instance = setmetatable({}, MenuHUD)
    return instance
end

function MenuHUD:Resume()
    if self.hud == 0 then return end
    UnpauseScene(self.hud)
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

function MenuHUD:Start()
    self.hud = CreateHUD("MenuHUD")
    if self.hud == 0 then return end

    HUD_SetButtonCallBack(self.hud, "ContinueButton", Resume)
    HUD_SetButtonCallBack(self.hud, "QuitButton", Quit)
    HUD_SetButtonCallBack(self.hud, "SettingButton", GetToSettingMenu)
end

function MenuHUD:Open()
    if self.hud ~= 0 then
        PauseScene(self.hud)
        ShowHUD(self.hud)
    end
end

return MenuHUD