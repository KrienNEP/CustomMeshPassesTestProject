#pragma once

#include "SceneViewExtension.h"

class FCustomMeshPassSceneViewExtension : public FSceneViewExtensionBase
{
public:
	FCustomMeshPassSceneViewExtension(const FAutoRegister& AutoRegister);
	
	//~ Begin FSceneViewExtensionBase Interface
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void SubscribeToCustomMeshVisibilityRelevancePass(FCustomMeshRelevanceCallbackDelegateMap& InOutCustomRelevanceCallbacks) override;
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override;
	//~ End FSceneViewExtensionBase Interface
	
	bool CustomMeshVisibilityRelevance_RenderThread(EShadingPath ShadingPath, const FPrimitiveViewRelevance& ViewRelevance, const FMeshBatch* Mesh, const FPrimitiveSceneProxy* Proxy);
};
