#include "CustomMeshPassEngineSubsystem.h"

#include "SceneViewExtension.h"
#include "CustomMeshPassSceneViewExtension.h"

#define LOCTEXT_NAMESPACE "CustomMeshPassEngineSubsystem"

void UCustomMeshPassEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CustomMeshPassSceneViewExtension = FSceneViewExtensions::NewExtension<FCustomMeshPassSceneViewExtension>();
}

void UCustomMeshPassEngineSubsystem::Deinitialize()
{
    Super::Deinitialize();
}


#undef LOCTEXT_NAMESPACE // CustomMeshPassEngineSubsystem