local Input = require("Input")

local SettingMenuHUD = {}
SettingMenuHUD.__index = SettingMenuHUD

function SettingMenuHUD:New(self)
	local instance = setmetatable({}, SettingMenuHUD)

    --Volume
    instance.mainVolume         = 100
    instance.musicVolume        = 100
    instance.soundEffectVolume  = 100

    --Inputs (only stored in qwerty)
    instance.inputs =
    {
        forward       = 'W',            -- 'Z' in azerty
        left          = 'A',            -- 'Q' in azerty
        back          = 'S',
        right         = 'D',
        jump          = 'SPACE',
        swimUp        = 'Q',            -- 'A' in azerty
        swimDown      = 'E',
    }

    --Graphic
    instance.ShadowToggle = true

    return instance
end

function SettingMenuHUD:ToggleShadow()
    self.hud = CreateHUD("SettingHUD")
    if self.hud == 0 then return end

    self.ShadowToggle = not self.ShadowToggle

    HUD_SetText(self.hud,   "ShadowButton", tostring(instance.ShadowToggle))
end

function SettingMenuHUD:SetInput(inputName, hudElement)
    local pressedKey = "TEMP_KEY"
    pressedKey = Input.GetPressedKey()

    self.inputs[inputName] = pressedKey

    if self.hud ~= 0 then
        HUD_SetText(self.hud, hudElement, tostring(pressedKey))
    end
end

function SettingMenuHUD:ExitHUD()
    UnpauseScene(self.hud)
end

function SettingMenuHUD:Start()
    self.hud = CreateHUD("SettingHUD")
    if self.hud == 0 then return end

    HUD_SetButtonCallBack(self.hud, "ExitButton"
    function()
    self:ExitHUD()
    end)

    --Volume
    HUD_SetText(self.hud,   "VolumeMainData",           tostring(self.mainVolume))
    HUD_SetText(self.hud,   "VolumeMusicData",          tostring(self.musicVolume))
    HUD_SetText(self.hud,   "VolumeSoundEffectData",    tostring(self.soundEffectVolume))

    --Inputs
    HUD_SetText(self.hud,   "ForwardInputData",     self.inputs.forward)
    HUD_SetText(self.hud,   "LeftInputData",        self.inputs.left)
    HUD_SetText(self.hud,   "BackInputData",        self.inputs.back)
    HUD_SetText(self.hud,   "RightInputData",       self.inputs.right)
    HUD_SetText(self.hud,   "JumpInputData",        self.inputs.jump)
    HUD_SetText(self.hud,   "SwimUpInputData",      self.inputs.swimUp)
    HUD_SetText(self.hud,   "SwimDownInputData",    self.inputs.swimDown)

    HUD_SetButtonCallBack(self.hud, "ForwardInputData", 
    function() 
    self:SetInput("forward", "ForwardInputData") 
    end)
    HUD_SetButtonCallBack(self.hud, "LeftInputData", 
    function() 
    self:SetInput("left", "LeftInputData") 
    end)
    HUD_SetButtonCallBack(self.hud, "BackInputData", 
    function() 
    self:SetInput("back", "BackInputData") 
    end)
    HUD_SetButtonCallBack(self.hud, "RightInputData", 
    function() 
    self:SetInput("right", "RightInputData") 
    end)
    HUD_SetButtonCallBack(self.hud, "JumpInputData", 
    function() 
    self:SetInput("jump", "JumpInputData") 
    end)
    HUD_SetButtonCallBack(self.hud, "SwimUpInputData", 
    function() 
    self:SetInput("swimUp", "SwimUpInputData") 
    end)
    HUD_SetButtonCallBack(self.hud, "SwimDownInputData", 
    function() 
    self:SetInput("swimDown", "SwimDownInputData") 
    end)

    --Graphic
    HUD_SetText(self.hud,   "ShadowButton", tostring(self.ShadowToggle))

    HUD_SetButtonCallBack(self.hud, "ShadowButton", 
    function() 
    self:ToggleShadow() 
    end)
end

function SettingMenuHUD:Open()
    if self.hud ~= 0 then
        PauseScene(self.hud)
        ShowHUD(self.hud)
    end
end

return SettingMenuHUD