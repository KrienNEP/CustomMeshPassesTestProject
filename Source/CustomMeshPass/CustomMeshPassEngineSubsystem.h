#pragma once

#include "CoreMinimal.h"

#include "Subsystems/EngineSubsystem.h"
#include "CustomMeshPassEngineSubsystem.generated.h"

class FCustomMeshPassSceneViewExtension;

UCLASS()
class UCustomMeshPassEngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	
private:
	TSharedPtr<FCustomMeshPassSceneViewExtension, ESPMode::ThreadSafe> CustomMeshPassSceneViewExtension;
};
