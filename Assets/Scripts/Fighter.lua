local Object = require("Object")
local Input = require("Input")

local Fighter = Object:New()

function Fighter:New()
    local instance = { 
    -- component
    opponent = nil,
    -- variables
    hud = nil,
    barName = "",
    hp = 19,
    maxhp = 10,
    }
    setmetatable(instance, { __index = self })
    return instance
end

function Fighter:SetOpponent(opp)
    self.opponent = opp
end

function Fighter:SetHuD(hud, barName)
    self.hud = hud
    self.barName = barName
end

function Fighter:Start()
    self.hp = self.maxhp
    self:SetEnabled(false)
end

function Fighter:Update(dt)
    if Input.IsKeyDown("P") then
        self.opponent:TakeDamage(0.1)
    end
end

function Fighter:TakeDamage(damage)
    Log("Ouch")
    self.hp = self.hp - damage

    if self.hp <= 0 then
        Log("grlunnqrniqenivgiuveqpuvrmuirebigribvarz")
    end

    HUD_SetProgress(self.hud, self.barName, self.hp / self.maxhp)
end


return Fighter