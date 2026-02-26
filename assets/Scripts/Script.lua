local Script = {}

function Script:New()
    local inst = { value = 0 }
    setmetatable(inst, { __index = Script })
    return inst
end

function Script:Increment()
    self.value = self.value + 1
    print("Value:", self.value)
end

return Script