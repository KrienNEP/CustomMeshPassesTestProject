#include "ColorMaterialMeshProcessor.h"

#include "MaterialDomain.h"
#include "Materials/MaterialRenderProxy.h"
#include "MeshPassProcessor.inl"

class FColorMaterialVS : public FMeshMaterialShader
{
public:
	DECLARE_SHADER_TYPE(FColorMaterialVS, MeshMaterial);

	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		return (Parameters.MaterialParameters.MaterialDomain == MD_Surface);
	}

	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FMeshMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FColorMaterialVS() = default;
	FColorMaterialVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FMeshMaterialShader(Initializer)
	{}
};

IMPLEMENT_MATERIAL_SHADER_TYPE(, FColorMaterialVS, TEXT("/Game/Private/ColorDrawing.usf"), TEXT("MainVS"), SF_Vertex);

class FColorMaterialPS : public FMeshMaterialShader
{
public:
	DECLARE_SHADER_TYPE(FColorMaterialPS, MeshMaterial);

	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		return (Parameters.MaterialParameters.MaterialDomain == MD_Surface);
	}

	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FMeshMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FColorMaterialPS() = default;
	FColorMaterialPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FMeshMaterialShader(Initializer)
	{}
};

IMPLEMENT_MATERIAL_SHADER_TYPE(, FColorMaterialPS, TEXT("/Game/Private/ColorDrawing.usf"), TEXT("MainPS"), SF_Pixel);

FColorMaterialMeshProcessor::FColorMaterialMeshProcessor(const FScene* Scene,
                                                         ERHIFeatureLevel::Type InFeatureLevel,
                                                         const FSceneView* InViewIfDynamicMeshCommand,
                                                         FMeshPassDrawListContext* InDrawListContext)
	: FMeshPassProcessor(EMeshPass::Num, Scene, InFeatureLevel, InViewIfDynamicMeshCommand, InDrawListContext)
{
	PassDrawRenderState.SetBlendState(TStaticBlendState<CW_RGBA>::GetRHI());
	PassDrawRenderState.SetDepthStencilState(TStaticDepthStencilState<false, CF_Always>::GetRHI());
}

void FColorMaterialMeshProcessor::AddMeshBatch(const FMeshBatch& RESTRICT MeshBatch, uint64 BatchElementMask, const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy, int32 StaticMeshId)
{
	if (MeshBatch.bUseForMaterial)
	{
		const FMaterialRenderProxy* MaterialRenderProxy = MeshBatch.MaterialRenderProxy;
		while (MaterialRenderProxy)
		{
			const FMaterial* Material = MaterialRenderProxy->GetMaterialNoFallback(FeatureLevel);
			if (Material)
			{
				if (TryAddMeshBatch(MeshBatch, BatchElementMask, PrimitiveSceneProxy, StaticMeshId, *MaterialRenderProxy, *Material))
				{
					break;
				}
			}

			MaterialRenderProxy = MaterialRenderProxy->GetFallback(FeatureLevel);
		}
	}
}

bool FColorMaterialMeshProcessor::TryAddMeshBatch(
	const FMeshBatch& RESTRICT MeshBatch,
	uint64 BatchElementMask,
	const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy,
	int32 StaticMeshId,
	const FMaterialRenderProxy& MaterialRenderProxy,
	const FMaterial& Material)
{
	const FMeshDrawingPolicyOverrideSettings OverrideSettings = ComputeMeshOverrideSettings(MeshBatch);
	const ERasterizerFillMode MeshFillMode = ComputeMeshFillMode(Material, OverrideSettings);
	const ERasterizerCullMode MeshCullMode = ComputeMeshCullMode(Material, OverrideSettings);

	return Process(MeshBatch, BatchElementMask, StaticMeshId, PrimitiveSceneProxy, MaterialRenderProxy, Material, MeshFillMode, MeshCullMode);
}

bool FColorMaterialMeshProcessor::Process(
	const FMeshBatch& MeshBatch,
	uint64 BatchElementMask,
	int32 StaticMeshId,
	const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy,
	const FMaterialRenderProxy& RESTRICT MaterialRenderProxy,
	const FMaterial& RESTRICT MaterialResource,
	ERasterizerFillMode MeshFillMode,
	ERasterizerCullMode MeshCullMode)
{
	const FVertexFactory* VertexFactory = MeshBatch.VertexFactory;
	FVertexFactoryType* VertexFactoryType = VertexFactory->GetType();

	FMaterialShaderTypes ShaderTypes;
	ShaderTypes.AddShaderType<FColorMaterialVS>();
	ShaderTypes.AddShaderType<FColorMaterialPS>();

	FMaterialShaders Shaders;
	if (!MaterialResource.TryGetShaders(ShaderTypes, VertexFactoryType, Shaders))
	{
		// Skip rendering if any shaders missing
		return false;
	}

	TMeshProcessorShaders<
		FColorMaterialVS,
		FColorMaterialPS> CustomMeshPassShaders;
	Shaders.TryGetVertexShader(CustomMeshPassShaders.VertexShader);
	Shaders.TryGetPixelShader(CustomMeshPassShaders.PixelShader);

	FMeshMaterialShaderElementData ShaderElementData;
	ShaderElementData.InitializeMeshMaterialData(ViewIfDynamicMeshCommand, PrimitiveSceneProxy, MeshBatch, StaticMeshId, true);

	// Use BasePass sort key layout but replace the "Masked" highest priority bits with TranslucencySortPriority.
	FMeshDrawCommandSortKey SortKey = FMeshDrawCommandSortKey::Default;

	BuildMeshDrawCommands(
		MeshBatch,
		BatchElementMask,
		PrimitiveSceneProxy,
		MaterialRenderProxy,
		MaterialResource,
		PassDrawRenderState,
		CustomMeshPassShaders,
		MeshFillMode,
		MeshCullMode,
		SortKey,
		EMeshPassFeatures::Default,
		ShaderElementData);

	return true;
}
