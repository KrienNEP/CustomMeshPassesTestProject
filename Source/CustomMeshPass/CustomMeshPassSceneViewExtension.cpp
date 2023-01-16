#include "CustomMeshPassSceneViewExtension.h"

#include "ColorMaterialMeshProcessor.h"
#include "MeshPassProcessor.inl"
#include "RendererUtils.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "Engine/TextureRenderTarget2D.h"
#include "InstanceCulling/InstanceCullingContext.h"
#include "Materials/MaterialRenderProxy.h"

namespace
{
constexpr FCustomMeshPassId SomeCustomMeshPassId = 66;
}

BEGIN_SHADER_PARAMETER_STRUCT(FCustomMeshPassParameters, )
    SHADER_PARAMETER_STRUCT_INCLUDE(FViewShaderParameters, View)
    SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FInstanceCullingGlobalUniforms, InstanceCulling)
    RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

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


void FCustomMeshPassSceneViewExtension::SubscribeToCustomMeshVisibilityRelevancePass(FCustomMeshRelevanceCallbackDelegateMap& InOutCustomRelevanceCallbacks)
{
	InOutCustomRelevanceCallbacks.Add(SomeCustomMeshPassId, FCustomMeshRelevanceCallbackDelegate::CreateRaw(this, &FCustomMeshPassSceneViewExtension::CustomStaticMeshVisibilityRelevance_RenderThread));
}

void FCustomMeshPassSceneViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs)
{
	if (!View.State || !View.bIsViewInfo)
	{
		// Only do post processing on views that have state
	    return;
	}
	
	const FString RenderTargetName = "/Script/Engine.TextureRenderTarget2D'/Game/RT_CameraView.RT_CameraView'";
	UTextureRenderTarget2D* RenderTarget = LoadObject<UTextureRenderTarget2D>(nullptr, *RenderTargetName);
	if (RenderTarget == nullptr)
	{
		return;
	}

	auto RenderTargetGraphBuilder = RegisterExternalTexture(GraphBuilder, RenderTarget->GetRenderTargetResource()->GetTexture2DRHI(), TEXT("RT_CameraView_GraphBuilder"));

	auto GetShaderParameters = [](const FSceneView& View)
	{
		FViewShaderParameters Parameters;
		Parameters.View = View.ViewUniformBuffer;
		return Parameters;
	};
	
	auto* PassParameters = GraphBuilder.AllocParameters<FCustomMeshPassParameters>();
	PassParameters->View = GetShaderParameters(View);
	PassParameters->InstanceCulling = FInstanceCullingContext::CreateDummyInstanceCullingUniformBuffer(GraphBuilder);
	PassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetGraphBuilder, ERenderTargetLoadAction::EClear);
	
	AddDrawDynamicMeshPass(
		GraphBuilder,
		RDG_EVENT_NAME("CustomMeshPass_66"),
		PassParameters,
		View,
		View.UnscaledViewRect,
		[&View](FDynamicPassMeshDrawListContext* DynamicMeshPassContext)
		{
			FColorMaterialMeshProcessor PassMeshProcessor(
				View.Family->Scene->GetRenderScene(),
				View.GetFeatureLevel(),
		        &View,
		        DynamicMeshPassContext);

            const TArray<FCustomMeshPassMeshBatch>* MeshBatchesPtr = GetMeshBatchesForCustomMeshPass(SomeCustomMeshPassId, View);
			if (MeshBatchesPtr)
			{
				const TArray<FCustomMeshPassMeshBatch>& MeshBatches = *MeshBatchesPtr;

				for (int32 MeshBatchIndex = 0; MeshBatchIndex < MeshBatches.Num(); ++MeshBatchIndex)
				{


					const FMeshBatch* Mesh = MeshBatches[MeshBatchIndex].Mesh;
					const FPrimitiveSceneProxy* PrimitiveSceneProxy = MeshBatches[MeshBatchIndex].Proxy;
					constexpr uint64 DefaultBatchElementMask = ~0ull;

					PassMeshProcessor.AddMeshBatch(*Mesh, DefaultBatchElementMask, PrimitiveSceneProxy);
				}
			}
		}, true);
}

bool FCustomMeshPassSceneViewExtension::CustomStaticMeshVisibilityRelevance_RenderThread(EShadingPath ShadingPath, const FPrimitiveViewRelevance& ViewRelevance, const FMeshBatch* Mesh, const FPrimitiveSceneProxy* Proxy)
{
	if (ShadingPath != EShadingPath::Deferred)
	{
		return false;
	}

	if (ViewRelevance.bDrawRelevance)
	{
		// Add all materials with this specific material name
		return Mesh->MaterialRenderProxy->GetMaterialName() == "MI_Red";
	}

	return false;
}
