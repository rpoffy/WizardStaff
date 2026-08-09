using UnrealBuildTool;

public class WizardStaff : ModuleRules
{
	public WizardStaff(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"Slate",
			"SlateCore",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"ProceduralMeshComponent"
		});

		DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
	}
}
