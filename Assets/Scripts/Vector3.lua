local Vector3 = {}

-- Constructor
function Vector3.New(x, y, z)
    return Vector3_New(x, y, z)
end

-- Methods
function Vector3:Normalized()
    return Vector3_Normalized(self)
end

return Vector3