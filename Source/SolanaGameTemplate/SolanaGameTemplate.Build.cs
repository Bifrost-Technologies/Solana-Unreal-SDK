// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SolanaGameTemplate : ModuleRules
{
	public SolanaGameTemplate(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "HTTP", "Json", "JsonUtilities", "InputCore", "HeadMountedDisplay", "EnhancedInput" });
	}
}
