local Object = require("Object")
 
local FinishStar = Object:New()
 
function FinishStar:New(self)
    local inst = {

     LoadTime = 1,
     TargetScene = "",
     start = false,
     accumulatedTime = 0,
     ScoreValue = 100,
    }

    inst.__exposed = {
         "LoadTime",
         "TargetScene",
         "ScoreValue",
    }

    setmetatable(inst, { __index = FinishStar })
    return inst
end
 
function FinishStar:Start()
    self:AddTag("FinishStar")
end
 
function FinishStar:Update(dt)
    if self.start then
        self.accumulatedTime = self.accumulatedTime + dt
        if self.accumulatedTime > self.LoadTime then
            LoadScene(self.TargetScene)
        end
    end
end
 
function FinishStar:OnTriggerEnter(col)
    local obj = col:GetOwner()
    if not obj:HasTag("Player") and not self.start then return end
    
    local player = Cast(obj:GetName(), "Player")
    if player == nil then return end
    player:UpdateScore(self.ScoreValue)
    self.start = true
end
 
return FinishStar

