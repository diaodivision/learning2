// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class learning2 : ModuleRules
{
    public learning2(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "GameplayTags",
            "GameplayAbilities",
            "GameplayTasks",
            "FogOfWar",
            "RewindSystem",
            "AIModule",
            "NavigationSystem",
            "Slate",
            "SlateCore",
            "Niagara",
            "WorldPauseSystem",
            "TopDownCameraSystem",
            "DeveloperSettings"
        });


        PrivateDependencyModuleNames.AddRange(new string[] { "UMG" });

        // ���ӹ�������·��
        PublicIncludePaths.AddRange(new string[] {
            "learning2",

            "learning2/Components",
            "learning2/Construction",
            "learning2/DataTable",
            "learning2/Gameplay",
            "learning2/HUD",
            "learning2/UI",
            "learning2/Utils",
            "learning2/Weapon",
        });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
