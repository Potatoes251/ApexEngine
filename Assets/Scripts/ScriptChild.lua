local Base = require("BaseObject")

local M = Base:New()

function M:Start()
    print("Child Start")
end

function M:Update(dt)
end

return M