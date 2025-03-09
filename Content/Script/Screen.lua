local M = {}

function M.PrintToScreen(text, color, duration)
    color = color or UE.FLinearColor(1, 1, 1, 1)
    duration = duration or 100
    UE.UKismetSystemLibrary.PrintString(nil, text, true, false, color, duration)
end

-- function M.Vector()
-- end

return M
