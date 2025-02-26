--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--
---@type BP_Vector_C
local M = UnLua.Class()

require("LuaPanda").start("127.0.0.1", 8818)

local Screen = require("Screen")

-- function M:Initialize(Initializer)
-- end

-- function M:UserConstructionScript()
-- end

function M:ReceiveBeginPlay()
    local Location = self:K2_GetActorLocation()
    Screen.PrintToScreen("Luhz " .. UE.UKismetStringLibrary.Conv_VectorToString(Location))
    -- self.SetActorTickEnabled(self, true)
end

-- function M:ReceiveEndPlay()
-- end

function M:ReceiveTick()
    -- local Location = UE.FVector()
    local Location = self:K2_GetActorLocation()
    Screen.PrintToScreen("Luhz " .. UE.UKismetStringLibrary.Conv_VectorToString(Location))
end

-- function M:ReceiveAnyDamage(Damage, DamageType, InstigatedBy, DamageCauser)
-- end

-- function M:ReceiveActorBeginOverlap(OtherActor)
-- end

-- function M:ReceiveActorEndOverlap(OtherActor)
-- end

return M
