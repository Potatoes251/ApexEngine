local Component = require("Component")

local Mesh = {}
Mesh.__index = Mesh
setmetatable(Mesh, { __index = Component })

-- Constructor
function Mesh:New(ptr)
    local instance = Component.New(self, ptr)
    return instance
end

-- Public function: fetch controller for an object
function Mesh.Get(obj)
    local ptr = GetMesh_Internal(obj)  -- C++ function returns pointer
    if not ptr then
        return nil
    end
    return Mesh:New(ptr)
end

-- Methods
function Mesh:SetUniform(name, value)
    return SetUniform_Internal(self, name, value)
end

return Mesh