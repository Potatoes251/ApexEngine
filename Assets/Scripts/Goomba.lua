local CharacterController = require("CharacterController")
local ViewComp = require("ViewComponent")
local Collider = require("Collider")
local Object = require("Object")
local Input = require("Input")
local Vector3 = require("Vector3")

local Goomba = Object:New()

function Goomba:New()
    local instance = { 
     -- component
    cc = nil, 
    view = nil,
    -- variables
    }
    setmetatable(instance, { __index = self })
    return instance
end

function Goomba:Start()
    self.cc = CharacterController.Get(self)
    self.view = ViewComp.Get(self)
    if self.view ~= nil then
        Log("Hello bitches")
    end
end


function Goomba:Update()
    if self.view:SeesTag("Player") then
        Log("Hello bitches")
    end
end

return Goomba