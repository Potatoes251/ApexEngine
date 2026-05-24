local SpawnManager = require("SpawnManager")
local SpawnPoint = {}
SpawnPoint.__index = SpawnPoint

function SpawnPoint:New(self)
	local instance = setmetatable({}, self)
    return instance
end

function SpawnPoint:OnTriggerEnter()	
	local NewSpawnPoint = GetPosition_Internal(self)
	if NewSpawnPoint ~= nil then
		SpawnManager:SetSpawnPoint(NewSpawnPoint)	--Get the position
	end
end

return SpawnPoint