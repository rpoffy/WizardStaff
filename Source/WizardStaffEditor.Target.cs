using UnrealBuildTool;
using System.Collections.Generic;

public class WizardStaffEditorTarget : TargetRules
{
	public WizardStaffEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("WizardStaff");
	}
}
