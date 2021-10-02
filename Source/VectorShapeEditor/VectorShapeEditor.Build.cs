//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

using UnrealBuildTool;

public class VectorShapeEditor : ModuleRules
{
	public VectorShapeEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		/*PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;*/   PCHUsage = ModuleRules.PCHUsageMode.NoPCHs; bUseUnity = false;

		PrivateIncludePaths.Add("VectorShapeEditor/Private");

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
                "InputCore",
				"Engine",
				"Slate",
				"SlateCore",
                "UnrealEd",
                "PropertyEditor",
                "EditorStyle",
                "ViewportInteraction",
                "ComponentVisualizers",        
                "RenderCore",
                "RHI",
                "RawMesh",
                "AssetTools",
                "AssetRegistry",
                "Projects",
                "AppFramework",

				// ... add private dependencies that you statically link with here ...	
                 "VectorShape",
            }
			);
	}
}
