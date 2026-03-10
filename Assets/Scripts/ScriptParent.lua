local Base = require("BaseObject")

local M = Base:New()

function M:Start()
    print("Parent Start")

    print("Reparent on self:", self.Reparent)
print("Reparent on other:", self.other.Reparent)

    self.other:Reparent(self)

    self:RemoveChild(self.other);
end

function M:Update(dt)
end

return M