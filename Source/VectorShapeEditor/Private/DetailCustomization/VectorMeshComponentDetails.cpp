//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#include "DetailCustomization/VectorMeshComponentDetails.h"

#include "Modules/ModuleManager.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Application/SlateWindowHelper.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "AssetRegistryModule.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets//Input/SComboButton.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBox.h"

#include "Engine/StaticMesh.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "RawMesh.h"
#include "Misc/PackageName.h"
#include "Dialogs/DlgPickAssetPath.h"
#include "ScopedTransaction.h"

#include "VectorMeshComponent.h"
#include "VectorShapeEditorHelpers.h"
#include "VectorSplineComponent.h"
#include "VectorShapeActor.h"

#include "VectorWidget/SlateVectorShapeData.h"

#define LOCTEXT_NAMESPACE "VectorMeshComponentDetails"

TSharedRef<IDetailCustomization> FVectorMeshComponentDetails::MakeInstance()
{
	return MakeShareable(new FVectorMeshComponentDetails);
}

void FVectorMeshComponentDetails::CustomizeDetails( IDetailLayoutBuilder& DetailBuilder )
{
	// Cache set of selected things
	SelectedObjectsList = DetailBuilder.GetSelectedObjects();

	if (SelectedObjectsList.Num() > 1)
	{
		return;
	}
	

	IDetailCategoryBuilder& SlateVectorDataCategory = DetailBuilder.EditCategory("Vector Shape Slate Data", FText::GetEmpty(), ECategoryPriority::Important);
	IDetailCategoryBuilder& VectorShapeSplinesCategory = DetailBuilder.EditCategory("Vector Spline", FText::GetEmpty(), ECategoryPriority::Important);



	SlateVectorDataCategory.AddCustomRow(FText::GetEmpty(), false)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2)
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
					[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton.Primary")
					.OnClicked(this, &FVectorMeshComponentDetails::ClickedOnSaveData)
					.IsEnabled(this, &FVectorMeshComponentDetails::CanSaveData)
					.VAlign(VAlign_Center)
					.Content()

					[
								SNew(SHorizontalBox)
								.Clipping(EWidgetClipping::ClipToBounds)
								+ SHorizontalBox::Slot()
									.VAlign(VAlign_Center)
									.Padding(2.f)
									.AutoWidth()
									[
										SNew(SImage)
										.Image(FAppStyle::Get().GetBrush("Icons.Save"))
									]

									+ SHorizontalBox::Slot()
									.VAlign(VAlign_Center)
									.Padding(2.f)
									.AutoWidth()
									[
										SNew(STextBlock)
										.Text(LOCTEXT("SaveDataToAsset", "Save Data To Asset"))
										.ToolTipText(LOCTEXT("SaveDataToAsset_Tooltip", "Save VectorShape Mesh Data To Asset."))
										.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
										//.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
									]
					]
					]
				]
		];

	VectorShapeSplinesCategory.AddCustomRow(FText::GetEmpty(), false)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2.f)
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
					[
						SNew(SComboButton)
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
						.ContentPadding(2)
						.ButtonContent()
						[
	
										SNew(SHorizontalBox)
										.Clipping(EWidgetClipping::ClipToBounds)
										+ SHorizontalBox::Slot()
											.VAlign(VAlign_Center)
											.Padding(2.f)
											.AutoWidth()
											[
												SNew(SImage)
												.Image(FAppStyle::Get().GetBrush("Icons.Plus"))
											]

											+ SHorizontalBox::Slot()
											.VAlign(VAlign_Center)
											.Padding(2.f)
											.AutoWidth()
											[
												SNew(STextBlock)
												.Text(LOCTEXT("NewSpline", "Spline"))
												.ToolTipText(LOCTEXT("NewSpline_ToolTip", "Add New Spline Component"))
												.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
												//.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
											]
						
						]
						.MenuContent()
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.Padding(2.0f)
								[
									SNew(SButton)
									.ButtonStyle(FEditorStyle::Get(), "FlatButton.Primary")
									.OnClicked(this, &FVectorMeshComponentDetails::ClickedOnAddNewSplineComponent, true)
									.Content()
									[
										SNew(STextBlock)
										.Text(LOCTEXT("Polygon", "(+) Polygon"))
										.ToolTipText(LOCTEXT("Polygon_ToolTip", "Add New Polygon"))
										.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
									]
								]
							+ SVerticalBox::Slot()
							.Padding(2.0f)
								[
									SNew(SButton)
									.ButtonStyle(FEditorStyle::Get(), "FlatButton.Warning")
									.OnClicked(this, &FVectorMeshComponentDetails::ClickedOnAddNewSplineComponent,false)
									.Content()
									[
										SNew(STextBlock)
										.Text(LOCTEXT("Line", "(-) Line"))
										.ToolTipText(LOCTEXT("Line_ToolTip", "Add New Line"))
										.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
									]
								]
						]
					]
				]
			+ SHorizontalBox::Slot()
			.Padding(2.f)
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
						[
						SNew(SButton)
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Danger")
						.OnClicked(this, &FVectorMeshComponentDetails::OnDeleteAllSpline)
						.IsEnabled(this, &FVectorMeshComponentDetails::CanDeleteAllSplines)
						.VAlign(VAlign_Center)
						.Content()
							[
									SNew(SHorizontalBox)
									.Clipping(EWidgetClipping::ClipToBounds)
									+ SHorizontalBox::Slot()
										.VAlign(VAlign_Center)
										.Padding(2.f)
										.AutoWidth()
										[
											SNew(SImage)
											.Image(FAppStyle::Get().GetBrush("Icons.X"))
										]

										+ SHorizontalBox::Slot()
										.VAlign(VAlign_Center)
										.Padding(2.f)
										.AutoWidth()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("DeleteSplines", "Delete Splines"))
											.ToolTipText(LOCTEXT("DeleteSplines_Tooltip", "Force Rebuild VectorShape Mesh & Collision."))
											.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
											//.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
										]
							]
					]
				]
		];


	
	
}

FReply FVectorMeshComponentDetails::ClickedOnSaveData()
{

	if (AVectorShapeActor* VectorShapeActor = GetSelectedVectorShapeActor())
	{
		if (USlateVectorShapeData* VectorShapeSlateVectorArtData = VectorShapeActor->SlateVectorData)
		{
			if (UVectorMeshComponent* VectorShapeMeshComp = VectorShapeActor->GetMeshComponent())
			{
				bool bDrawMesh = VectorShapeActor->bDrawMesh;
				const FScopedTransaction Transaction(LOCTEXT("ForceRebuildVectorShapeMesh", "Force Rebuild VectorShape Mesh"));

				if (!bDrawMesh)
				{
					VectorShapeActor->bDrawMesh = true;
					VectorShapeActor->RebuildVectorShapeMesh();
				}

				if (FVectorShapeMeshSection* MeshSection = VectorShapeMeshComp->GetVectorMeshSection(0))
				{
					if (MeshSection->IsValid())
					{
						
						VectorShapeSlateVectorArtData->Modify();
						VectorShapeSlateVectorArtData->InitFromVectorCustomMesh(MeshSection, VectorShapeActor->WorldSize);
						VectorShapeSlateVectorArtData->PostEditChange();
						FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
						PropertyModule.NotifyCustomizationModuleChanged();
					}
				}

				if (!bDrawMesh)
				{
					VectorShapeActor->bDrawMesh = false;
					VectorShapeActor->RebuildVectorShapeMesh();
				}
			}
		}
	}




	return FReply::Handled();
}

UVectorMeshComponent* FVectorMeshComponentDetails::GetSelectedVectorMeshComp() const
{
	// Find first selected valid VectorShapeMeshComp
	UVectorMeshComponent* VectorShapeMeshComp = nullptr;
	for (const TWeakObjectPtr<UObject>& Object : SelectedObjectsList)
	{
		AVectorShapeActor* VectorShapeActor = Cast<AVectorShapeActor>(Object.Get());
		// See if this one is good
		if (IsValid(VectorShapeActor) && !VectorShapeActor->IsUnreachable())
		{
			VectorShapeMeshComp = VectorShapeActor->GetMeshComponent();
			break;
		}
	}

	return VectorShapeMeshComp;
}



AVectorShapeActor* FVectorMeshComponentDetails::GetSelectedVectorShapeActor() const
{
	UVectorMeshComponent* VectorShapeMeshComp = nullptr;
	for (const TWeakObjectPtr<UObject>& Object : SelectedObjectsList)
	{
		AVectorShapeActor* VectorShapeActor = Cast<AVectorShapeActor>(Object.Get());
		// See if this one is good
		if (IsValid(VectorShapeActor) && !VectorShapeActor->IsUnreachable())
		{
			return VectorShapeActor;
		}
	}
	return nullptr;
}

bool FVectorMeshComponentDetails::ConvertToStaticMeshEnabled() const
{
	return GetSelectedVectorMeshComp() != nullptr;
}


bool FVectorMeshComponentDetails::CanSaveData() const
{
	if (const AVectorShapeActor* VectorShapeActor = GetSelectedVectorShapeActor())
	{
		return VectorShapeActor->SlateVectorData != nullptr && VectorShapeActor->GetMeshComponent() != nullptr;
	}
	return false;
}

FReply FVectorMeshComponentDetails::OnDeleteAllSpline()
{
	if (AVectorShapeActor* VectorShapeActor = GetSelectedVectorShapeActor())
	{
		VectorShapeEditorHelpers::ClearAllSplineComponents(VectorShapeActor);
	}
	return FReply::Handled();
}

bool FVectorMeshComponentDetails::CanDeleteAllSplines() const
{
	if (const AVectorShapeActor* VectorShapeActor = GetSelectedVectorShapeActor())
	{
		TArray<UActorComponent*> SplineComponents;
		VectorShapeActor->GetComponents(UVectorSplineComponent::StaticClass(), SplineComponents);
		return SplineComponents.Num() > 0;
	}
	return false;
}

FReply FVectorMeshComponentDetails::ClickedOnAddNewSplineComponent(bool bIsPolygon)
{
	UVectorMeshComponent* VectorShapeMeshComp = GetSelectedVectorMeshComp();
	if (IsValid(VectorShapeMeshComp) && !VectorShapeMeshComp->IsUnreachable())
	{
		if (AVectorShapeActor* VectorShapeActor = Cast<AVectorShapeActor>(VectorShapeMeshComp->GetOwner()))
		{
			VectorShapeEditorHelpers::AddNewSplineComponent(VectorShapeActor, bIsPolygon);
			VectorShapeActor->RebuildVectorShapeMesh();
		}
	}

	FSlateApplication::Get().DismissAllMenus();
	return FReply::Handled();
}




#undef LOCTEXT_NAMESPACE
