local M = {}

-- Constructor
function M:New()
    local instance = {}
    setmetatable(instance, { __index = self })

    -- State variables for movement
    instance.timer = 0
    instance.speed = 2.0    -- How fast it moves
    instance.distance = 5.0 -- How far it travels
    
    instance.transform = {}

    instance.__exposed = {
        "speed",
        "distance"
    }

    local inst = instance
    function instance.transform:SetPosition(x, y, z)
        SetPosition_Internal(inst, x, y, z)
    end

    return instance
end

function M:Start()
    print("Moving Platform Initialized")
end

-- Called every frame
function M:Update(dt)
    self.timer = self.timer + dt
    
    -- Calculate a smooth back-and-forth offset using a sine wave
    -- Formula: offset = sin(time * speed) * distance
    local offset = math.sin(self.timer * self.speed) * self.distance
    
    -- Update position (moving along the X-axis in this example)
    self.transform:SetPosition(offset, 0, 0)
end

return M