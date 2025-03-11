--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

require("LuaPanda").start("127.0.0.1", 8818)

---@type BP_CollisionActor_C
local M = UnLua.Class()

-- function M:Initialize(Initializer)
-- end

-- function M:UserConstructionScript()
-- end

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

function M:ReceiveShowAABBBox()
    local Mesh = self.SM_Cube
    local Origin = UE.FVector()
    local BoxExtent = UE.FVector()
    UE.UKismetSystemLibrary.GetComponentBounds(Mesh, Origin, BoxExtent, 0.0)
    -- BoxExtent =  + BoxExtent
    UE.UKismetSystemLibrary.DrawDebugBox(
        self:GetWorld(),
        Origin,
        BoxExtent + UE.FVector(5.0, 5.0, 5.0),
        self.BP_BoundColor,
        UE.FRotator(0.0, 0.0, 0.0),
        self.BP_BoundDuration,
        0.0
    )
    -- require("Screen").PrintToScreen("Rotating", UE.FLinearColor(1, 1, 1, 1), 1.5)
end

function M:ReceiveCreateTimer(bIsAdd)
    if bIsAdd then
        self.BP_CurTimer = UE.UKismetSystemLibrary.K2_SetTimerDelegate({self, M.AddRotation}, 0.005, true)
    else
        self.BP_CurTimer = UE.UKismetSystemLibrary.K2_SetTimerDelegate({self, M.SubRotation}, 0.005, true)
    end
end

function M:ReceiveRemoveTimer()
    UE.UKismetSystemLibrary.K2_ClearTimerHandle(self, self.BP_CurTimer)
end

function M:AddRotation()
    self:K2_AddActorWorldRotation(self.BP_CurAbsRotater, false, UE.FHitResult(), false)
    -- self:ReceiveShowAABBBox()
end

function M:SubRotation()
    self:K2_AddActorWorldRotation(self:GetMinusAbsRotaer(), false, UE.FHitResult(), false)
    -- self:ReceiveShowAABBBox()
end

function M:GetMinusAbsRotaer()
    return UE.UKismetMathLibrary.Multiply_RotatorFloat(self.BP_CurAbsRotater, -1)
end

return M
