--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

require("LuaPanda").start("127.0.0.1", 8818)

---@type BP_Constraint_C
local M = UnLua.Class("BP_CollisionActor_C")

-- function M:Initialize(Initializer)
-- end

function M:UserConstructionScript()
    self.Super.UserConstructionScript(self)
    self.Constraint_Cube.SetVectorParameterValueOnMaterials(
        UE.FName("SurfaceColor"),
        UE.UKismetMathLibrary.Conv_LinearColorToVector(self.BP_ConstraintColor)
    )
end

-- function M:ReceiveBeginPlay()
-- end

-- function M:ReceiveEndPlay()
-- end

-- function M:ReceiveTick(DeltaSeconds)
-- end

-- function M:ReceiveAnyDamage(Damage, DamageType, InstigatedBy, DamageCauser)
-- end

-- function M:ReceiveActorBeginOverlap(OtherActor)
-- end

-- function M:ReceiveActorEndOverlap(OtherActor)
-- end

return M
