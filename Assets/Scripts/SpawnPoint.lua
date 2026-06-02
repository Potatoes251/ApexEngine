local SpawnManager = require("SpawnManager")
local Object = require("Object")

local SpawnPoint = {}
SpawnPoint.__index = SpawnPoint

function SpawnPoint:New(self)
	local instance = setmetatable({}, self)
    return instance
end

function SpawnPoint:OnTriggerEnter(col)	
	local obj = col:GetOwner()
	if obj:HasTag('Player') then
		local NewSpawnPoint = self:GetPosition()
		if NewSpawnPoint ~= nil then
			SpawnManager:SetSpawnPoint(NewSpawnPoint)
		end
	end
end

return SpawnPoint