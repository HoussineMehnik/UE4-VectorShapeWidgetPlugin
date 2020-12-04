// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "VectorShapeEditorModule.h"
#include "Editor.h"

#include "ComponentVisualizer.h"
#include "SplineVisualizer/VectorSplineComponentVisualizer.h"
#include "VectorSplineComponent.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

#include "VectorShapeTypes.h"
#include "VectorShapeEditorHelpers.h"
#include "DetailCustomization/VectorMeshComponentDetails.h"
#include "VectorMeshComponent.h"

#include "PropertyEditorModule.h"
#include "VectorShapeActor.h"
#include "VectorShapeEditorStyle.h"
#include "SplineVisualizer/VectorMeshComponentVisualizer.h"


#include "EditorModeRegistry.h"


#include "VectorWidget/SlateVectorShapeData.h"
#include "DetailCustomization/SlateVectorShapeDataDetails.h"


#include "IAssetTools.h"
#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"
#include "TypeActions/AssetTypeActions_VectorShapeData.h"
#include "IAssetTypeActions.h"


#define LOCTEXT_NAMESPACE "FVectorShapeEditorModule"


void FVectorShapeEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Register Editor Style
	FVectorShapeEditorStyle::Initialize();

	//
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		TSharedRef<IAssetTypeActions> VectorDataAssetActions = MakeShareable(new FAssetTypeActions_VectorShapeData());
		AssetTools.RegisterAssetTypeActions(VectorDataAssetActions);
		CreatedAssetTypeActions.Add(VectorDataAssetActions);
	}

	// Register Visualizers
	if (GUnrealEd != nullptr)
	{
		TSharedPtr<FComponentVisualizer> Visualizer = MakeShareable(new FVectorSplineComponentVisualizer());
		GUnrealEd->RegisterComponentVisualizer(UVectorSplineComponent::StaticClass()->GetFName(), Visualizer);
		if (Visualizer.IsValid())
		{
			Visualizer->OnRegister();
		}

		Visualizer.Reset();
		Visualizer = MakeShareable(new FVectorMeshComponentVisualizer());
		GUnrealEd->RegisterComponentVisualizer(UVectorMeshComponent::StaticClass()->GetFName(), Visualizer);
		if (Visualizer.IsValid())
		{
			Visualizer->OnRegister();
		}
	}

	//
	{
		FVectorShapeEditorDelegates::OnCreateVectorMeshDelegate.AddRaw(this, &FVectorShapeEditorModule::OnCreateVectorShapeMesh);
	}

	//
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.RegisterCustomClassLayout(AVectorShapeActor::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FVectorMeshComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(USlateVectorShapeData::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FSlateVectorShapeDataDetails::MakeInstance));

	}
}

void FVectorShapeEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (UObjectInitialized())
	{
		
		
		if (GUnrealEd != nullptr)
		{
			GUnrealEd->UnregisterComponentVisualizer(UVectorSplineComponent::StaticClass()->GetFName());
			GUnrealEd->UnregisterComponentVisualizer(UVectorMeshComponent::StaticClass()->GetFName());

		}

		
		{
			FVectorShapeEditorDelegates::OnCreateVectorMeshDelegate.RemoveAll(this);
		}

		{
			if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
			{
				IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
				for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
				{
					AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
				}
			}
			CreatedAssetTypeActions.Empty();
		}

		

		// Unregister Style
		FVectorShapeEditorStyle::Shutdown();


	}
}

void FVectorShapeEditorModule::OnCreateVectorShapeMesh(AVectorShapeActor* VectorShapeActor)
{
	VectorShapeEditorHelpers::OnCreateVectorMesh(VectorShapeActor);
}



#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVectorShapeEditorModule, VectorShapeEditor)