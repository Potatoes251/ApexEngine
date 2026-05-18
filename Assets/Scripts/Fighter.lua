

local Fighter = {}

function Fighter:New()
    local instance = { 
    -- component
    opponent = nil,
    -- variables
    hp = 19,
    maxhp = 10,
    }
    setmetatable(instance, { __index = self })
    return instance
end

function Fighter:SetOpponent(opp)
    self.opponent = opp
    Log("hello")
end


function Fighter:Start()
    self.hp = self.maxhp

end


return Fighter