// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/BFLCommon.h"


bool UBFLCommon::GetAttributeValue(UAbilitySystemComponent* ASC, FGameplayAttribute Attribute, float& BaseValue, float& CurValue)
{
	bool Tag = false;
	if (ASC != nullptr)
	{
		Tag = true;
		BaseValue = ASC->GetNumericAttributeBase(Attribute);
		CurValue = ASC->GetNumericAttribute(Attribute);
	}
	return Tag;
}
