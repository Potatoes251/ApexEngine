


local Fighter = {}

function Fighter:New()
    local instance = { 
     -- component
    oponent = nil,
    -- variables
    hp = 19,
    maxhp = 10,
    }
    setmetatable(instance, { __index = Player })
    return instance
end


function Fighter:Start()
    self.hp = self.maxhp

end


return Fighter