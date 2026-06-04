local Empty = {}

function Empty:New()
    local instance = {}
    setmetatable(instance, { __index = self })
    return instance
end

function Empty:OnStart()
    self:SetEnabled(false)
end

return Empty