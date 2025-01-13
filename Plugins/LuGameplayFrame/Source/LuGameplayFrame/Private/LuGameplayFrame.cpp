// Copyright Epic Games, Inc. All Rights Reserved.

#include "LuGameplayFrame.h"

#define LOCTEXT_NAMESPACE "FLuGameplayFrameModule"

void FLuGameplayFrameModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FLuGameplayFrameModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLuGameplayFrameModule, LuGameplayFrame)