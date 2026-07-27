// Copyright Epic Games, Inc. All Rights Reserved.

#include "FogOfWar.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FFogOfWarModule"

void FFogOfWarModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("FogOfWar"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/FogOfWar"), PluginShaderDir);

	//// 获取插件目录
	//TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("FogOfWar"));
	//if (Plugin.IsValid())
	//{
	//	FString PluginShaderDir = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Shaders"));
	//	AddShaderSourceDirectoryMapping(TEXT("/FogOfWar"), PluginShaderDir);

	//	UE_LOG(LogTemp, Log, TEXT("FogOfWar module loaded. Shader directory: %s"), *PluginShaderDir);
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("Failed to find FogOfWar plugin"));
	//}
}

void FFogOfWarModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFogOfWarModule, FogOfWar)