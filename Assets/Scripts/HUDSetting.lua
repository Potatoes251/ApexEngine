local SettingMenuHUD = {}
SettingMenuHUD.__index = SettingMenuHUD

function SettingMenuHUD:New(self)
	local instance = setmetatable({}, SettingMenuHUD)

    --Volume
    instance.mainVolume         = 100
    instance.musicVolume        = 100
    instance.soundEffectVolume  = 100

    --Inputs
    instance.inputs =
    {
        forward       = 'Z'
        left          = 'Q'
        back          = 'S'
        right         = 'D'
        jump          = 'SPACE'
        swimUp        = 'A'
        swimDown      = 'E'
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
    --TODO: get the input 

    self.inputs[inputName] = pressedKey

    if self.hud ~= 0 then
        HUD_SetText(self.hud, hudElement, tostring(pressedKey))
    end
end

function SettingMenuHUD:Start()
    self.hud = CreateHUD("SettingHUD")
    if self.hud == 0 then return end

    --Volume
    HUD_SetText(self.hud,   "VolumeMainData", tostring(instance.mainVolume))
    HUD_SetText(self.hud,   "VolumeMusicData", tostring(instance.musicVolume))
    HUD_SetText(self.hud,   "VolumeSoundEffectData", tostring(instance.soundEffectVolume))

    --Inputs
    HUD_SetText(self.hud,   "ForwardInputData", instance.forwardInput)
    HUD_SetText(self.hud,   "LeftInputData", instance.leftInput)
    HUD_SetText(self.hud,   "BackInputData", instance.backInput)
    HUD_SetText(self.hud,   "RightInputData", instance.rightInput)
    HUD_SetText(self.hud,   "JumpInputData", instance.jumpInput)
    HUD_SetText(self.hud,   "SwimUpInputData", instance.swimUpInput)
    HUD_SetText(self.hud,   "SwimDownInputData", instance.swimDownInput)

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
    HUD_SetText(self.hud,   "ShadowButton", tostring(instance.ShadowToggle))

    HUD_SetButtonCallBack(self.hud, "ShadowButton", ToggleShadow)
end