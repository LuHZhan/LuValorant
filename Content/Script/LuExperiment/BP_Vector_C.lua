--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--
---@type BP_Vector_C
local M = UnLua.Class()

require("LuaPanda").start("127.0.0.1",8818)

local Screen = require("Screen")

-- function M:Initialize(Initializer)
-- end

-- function M:UserConstructionScript()
-- end

function M:ReceiveBeginPlay()
    local msg = [[Luhz]]
    print(msg)
    Screen.PrintToScreen(msg)
    -- self.SetActorTickEnabled(self, true)
end

-- function M:ReceiveEndPlay()
-- end

function M:ReceiveTick(DeltaSeconds)
    local Location = UE.FVector();
    self:K2_GetActorLocation(Location)
    Screen.PrintToScreen(UE.UKismetStringLibrary.Conv_VectorToString(Location))
end

-- function M:ReceiveAnyDamage(Damage, DamageType, InstigatedBy, DamageCauser)
-- end

-- function M:ReceiveActorBeginOverlap(OtherActor)
-- end

-- function M:ReceiveActorEndOverlap(OtherActor)
-- end

return M
