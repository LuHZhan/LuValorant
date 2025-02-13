// Copyright Epic Games, Inc. All Rights Reserved.

#include "LuExperiment.h"

#define LOCTEXT_NAMESPACE "FLuExperimentModule"

void FLuExperimentModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FLuExperimentModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLuExperimentModule, LuExperiment)