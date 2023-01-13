// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildBase;
using UnrealBuildTool;

public class CustomMeshPass : ModuleRules
{
	public CustomMeshPass(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "RenderCore", "Renderer", "RHI" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        string RendererPrivatePath = Path.Combine(Unreal.EngineDirectory.FullName, "Source", "Runtime", "Renderer", "Private");

        PrivateIncludePaths.AddRange(
            new string[] {
                // Hack used in GoogleARCoreRendering plugin module, which is authored by Epic. I guess it's sort of legal?
                RendererPrivatePath
            }
        );

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
