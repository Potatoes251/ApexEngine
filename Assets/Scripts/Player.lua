local CharacterController = require("CharacterController")
local Input = require("Input")


local Player = {}

function Player:New()
    local instance = { cc = nil }
    setmetatable(instance, { __index = self })
    return instance
end

function Player:Start()
    self.cc = CharacterController.Get(self)
end

function Player:Update(dt)
    local x = Input.GetAxis("Horizontal")
    local z = Input.GetAxis("Vertical")
    local y = 0

    self.cc:Move(x, y, z)

    if (Input.IsKeyDown("Space")) then
        self.cc:Jump()
    end
end

return Player