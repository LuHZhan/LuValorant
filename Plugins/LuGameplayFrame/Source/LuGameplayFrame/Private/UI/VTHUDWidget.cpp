// Copyright 2024 Dan Kestranek.


#include "UI/VTHUDWidget.h"
#include "AttributeSet.h"

bool UVTHUDWidget::CallUpdateEvent_Implementation(FGameplayAttribute AS, float NewValue, float OldValue)
{
	bool Tag = false;
	if (AttributeFuncNameMap.Contains(AS))
	{
		const FName FuncName = AttributeFuncNameMap[AS];
		if (UFunction* Func = FindFunction(FuncName); Func != nullptr)
		{
			// 分配内存块
			void* Params = FMemory::Malloc(Func->ParmsSize, 16); // 16 字节对齐
			FMemory::Memzero(Params, Func->ParmsSize); // 初始化内存

			// 填充参数
			for (TFieldIterator<FProperty> It(Func); It; ++It)
			{
				FProperty* Property = *It;
				// 设置 float 
				*Property->ContainerPtrToValuePtr<float>(Params) = NewValue;
				*Property->ContainerPtrToValuePtr<float>(Params) = OldValue;
			}

			ProcessEvent(Func, Params);
			// 释放内存
			FMemory::Free(Params);
			Tag = true;
		}
	}
	return Tag;
}
