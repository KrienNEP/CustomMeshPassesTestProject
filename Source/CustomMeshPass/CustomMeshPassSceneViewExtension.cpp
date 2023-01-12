#include "CustomMeshPassSceneViewExtension.h"

#include "Materials/MaterialRenderProxy.h"

FCustomMeshPassSceneViewExtension::FCustomMeshPassSceneViewExtension(const FAutoRegister& AutoRegister)
    : FSceneViewExtensionBase(AutoRegister)
{
	FSceneViewExtensionIsActiveFunctor IsActiveFunctor;

	IsActiveFunctor.IsActiveFunction = [](const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext& Context)
	{
		// Only run for Game, Editor or PIE worlds
        if (const UWorld* CurrentWorld = Context.GetWorld())
		{
			return CurrentWorld->WorldType == EWorldType::Game
				|| CurrentWorld->WorldType == EWorldType::Editor
				|| CurrentWorld->WorldType == EWorldType::PIE;
		}
		return false;
	};

	IsActiveThisFrameFunctions.Add(IsActiveFunctor);
}


void FCustomMeshPassSceneViewExtension::SubscribeToCustomStaticMeshVisibilityRelevancePass(FCustomStaticMeshRelevanceCallbackDelegateMap& InOutCustomRelevanceCallbacks)
{
	static FCustomMeshPassId SomeCustomMeshPassId = 66;
	InOutCustomRelevanceCallbacks.Add(SomeCustomMeshPassId, FCustomStaticMeshRelevanceCallbackDelegate::CreateRaw(this, &FCustomMeshPassSceneViewExtension::CustomStaticMeshVisibilityRelevance_RenderThread));
}

void FCustomMeshPassSceneViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs)
{
	if (!View.State)
	{
		// Only do post processing on views that have state
	    return;
	}
	
	//const FViewInfo& ViewInfo = static_cast<const FViewInfo&>(View);
}

bool FCustomMeshPassSceneViewExtension::CustomStaticMeshVisibilityRelevance_RenderThread(EShadingPath ShadingPath, const FPrimitiveViewRelevance& ViewRelevance, const FMeshBatch* Mesh, const FPrimitiveSceneProxy* Proxy)
{
	if (ShadingPath != EShadingPath::Deferred || !ViewRelevance.bRenderInMainPass)
	{
		return false;
	}

	// Add all materials with this specific material name
	return Mesh->MaterialRenderProxy->GetMaterialName() == "MI_Red";
}
