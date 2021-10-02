//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

using UnrealBuildTool;

public class VectorShape : ModuleRules
{
	public VectorShape(ReadOnlyTargetRules Target) : base(Target)
	{
		/*PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;*/   PCHUsage = ModuleRules.PCHUsageMode.NoPCHs; bUseUnity = false;

		PrivateIncludePaths.Add("VectorShape/Private");

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				    "CoreUObject",
				    "Engine",

                    "RenderCore",
                    "RHI",

                    "SlateCore",
                    "Slate",
                    "UMG"
				// ... add private dependencies that you statically link with here ...	
			}
			);	
    }
}
