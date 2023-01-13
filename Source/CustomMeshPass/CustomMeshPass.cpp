// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomMeshPass.h"

#include "Modules/ModuleManager.h"

void FCustomMeshPassModule::StartupModule()
{
    const FString ProjectShaderDir = FPaths::Combine(FPlatformMisc::ProjectDir(), TEXT("Shaders"));
    AddShaderSourceDirectoryMapping(TEXT("/Game"), ProjectShaderDir);
}

IMPLEMENT_MODULE(FCustomMeshPassModule, CustomMeshPass)