local MovingPlatform = {}

-- Constructor
function MovingPlatform:New()
    local instance = {}
    setmetatable(instance, { __index = self })

    -- Exposed properties
    instance.Speed = 2.0
    instance.Distance = 5.0
    instance.MoveX = true
    instance.MoveY = false
    instance.MoveZ = false

    instance.__exposed = {
        "Speed",
        "Distance",
        "MoveX",
        "MoveY",
        "MoveZ"
    }

    -- State variables
    instance.Timer = 0
    instance.originX = 0
    instance.originY = 0
    instance.originZ = 0
    
    instance.transform = {}
    local inst = instance
    
    function instance.transform:SetPosition(x, y, z)
        SetPosition_Internal(inst, x, y, z)
    end

    function instance.transform:GetPosition()
        return GetPosition_Internal(inst)
    end

    return instance
end

function MovingPlatform:Start()
    -- Capture the initial editor position
    local x, y, z = self.transform:GetPosition()
    self.originX = x or 0
    self.originY = y or 0
    self.originZ = z or 0
end

function MovingPlatform:Update(dt)
    self.Timer = self.Timer + dt
    
    -- Calculate smooth 0 to 1 alpha
    local alpha = (math.sin(self.Timer * self.Speed) + 1) / 2
    local offset = alpha * self.Distance
    
    -- Determine movement per axis based on exposed booleans
    local curX = self.originX + (self.MoveX and offset or 0)
    local curY = self.originY + (self.MoveY and offset or 0)
    local curZ = self.originZ + (self.MoveZ and offset or 0)
    
    -- Apply the new position
    self.transform:SetPosition(curX, curY, curZ)
end

return MovingPlatform