// Copyright Epic Games, Inc. All Rights Reserved.

#include "LuExperimentBPLibrary.h"
#include "LuExperiment.h"
#include "StaticMeshAttributes.h"

ULuExperimentBPLibrary::ULuExperimentBPLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

float ULuExperimentBPLibrary::LuExperimentSampleFunction(float Param)
{
	return -1;
}

FVector ULuExperimentBPLibrary::GetStaticMeshApproxSize(UStaticMesh* InMesh)
{
	if (!InMesh)
	{
		return FVector::ZeroVector;
	}

	const FMeshDescription* MeshDescription = InMesh->GetMeshDescription(0);
	if (!MeshDescription)
	{
		return FVector::ZeroVector;
	}

	FStaticMeshConstAttributes Attributes(*MeshDescription);
	TVertexAttributesConstRef<FVector3f> VertexPositions = Attributes.GetVertexPositions();

	// 计算边界盒
	FBox MeshBox(ForceInit);
	for (const FVector3f& Position : VertexPositions.GetRawArray())
	{
		MeshBox += FVector(Position);
	}

	return MeshBox.GetSize();
}
