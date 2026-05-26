local SpawnManager = {}
SpawnManager.__index = SpawnManager

function SpawnManager:New(self)
    local instance = setmetatable({}, self)
    spawnPosition = nil    --Vector3
    return instance
end

function SpawnManager:SetSpawnPoint(newSpawnPoint)

    if newSpawnPoint ~= nil then
        self.spawnPosition = newSpawnPoint
    end
end

return SpawnManager