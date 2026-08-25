// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class FogOfWar : ModuleRules
{
    public FogOfWar(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // ���� Shaders Ŀ¼������·��
        string ShaderPublicPath = Path.Combine(ModuleDirectory, "../../Shaders/Public");
        PublicIncludePaths.Add(ShaderPublicPath);

        PublicIncludePaths.AddRange(
            new string[] {
				// ... add public include paths required here ...
			}
            );


        PrivateIncludePaths.AddRange(
            new string[] {
				// ... add other private include paths required here ...
			}
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "RenderCore",    // FRDGBuilder, FGlobalShader, AddShaderSourceDirectoryMapping
                "RHI",           // FRHICommandList, GMaxRHIFeatureLevel, GPU ��Դ����
                "Projects",      // IPluginManager�����ڶ�λ���·����
                "Landscape"
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Renderer",         // SetComputePipelineState, SetShaderParameters, GetGlobalShaderMap
                "TopDownCameraSystem",
                "DeveloperSettings"
            }
        );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );
    }
}
