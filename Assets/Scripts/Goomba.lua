local CharacterController = require("CharacterController")
local Collider = require("Collider")
local Object = require("Object")
local Input = require("Input")
local Vector3 = require("Vector3")

local Goomba = Object:New()

function Goomba:New()
    local instance = { 
     -- component
    cc = nil, 
    -- variables
    }
    setmetatable(instance, { __index = self })
    return instance
end

function Goomba:Start()
    self.cc = CharacterController.Get(self)
    if self.cc ~= nil then
        Log("Hello bitches")
    end
end


function Goomba:Update()

end

return Goomba