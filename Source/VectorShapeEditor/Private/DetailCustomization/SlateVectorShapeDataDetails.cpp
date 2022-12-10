//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#include "DetailCustomization/SlateVectorShapeDataDetails.h"
#include "Modules/ModuleManager.h"
#include "Misc/PackageName.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Application/SlateWindowHelper.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Engine/StaticMesh.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"

#include "RawMesh.h"

#include "Dialogs/DlgPickAssetPath.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SComboButton.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBox.h"

#include "Widgets/Layout/SScaleBox.h"
#include "EditorFontGlyphs.h"
#include "ScopedTransaction.h"

#include "VectorMeshComponent.h"
#include "VectorShapeEditorHelpers.h"
#include "VectorSplineComponent.h"
#include "VectorShapeActor.h"


#include "VectorShapeEditorStyle.h"
#include "VectorWidget/SVectorShapeWidget.h"
#include "VectorWidget/SlateVectorShapeData.h"
#include "VectorShapeEditorModule.h"
#include "VectorShapeTypes.h"

#define LOCTEXT_NAMESPACE "VectorShapeSlateVectorArtDataDetails"

FSlateVectorShapeDataDetails::FSlateVectorShapeDataDetails()
	: DetailBuilderPtr(nullptr)
{
	FVectorShapeEditorDelegates::OnVectorDataAssetChangedDelegate.AddRaw(this, &FSlateVectorShapeDataDetails::RefreshDetails);
}

FSlateVectorShapeDataDetails::~FSlateVectorShapeDataDetails()
{
	DetailBuilderPtr = nullptr;
	FVectorShapeEditorDelegates::OnVectorDataAssetChangedDelegate.RemoveAll(this);
}

TSharedRef<IDetailCustomization> FSlateVectorShapeDataDetails::MakeInstance()
{
	return MakeShareable(new FSlateVectorShapeDataDetails);
}

void FSlateVectorShapeDataDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilderPtr = &DetailBuilder;

	IDetailCategoryBuilder& VectorMeshInfoCategory = DetailBuilder.EditCategory("Data Info", FText::GetEmpty(), ECategoryPriority::Important);
	IDetailCategoryBuilder& SlateVectorDataCategory = DetailBuilder.EditCategory("Preview", FText::GetEmpty(), ECategoryPriority::Uncommon);
	TSharedPtr<SVectorShapeWidget> VectorShapeMeshWidget;


	VectorMeshInfoCategory.AddCustomRow(FText::GetEmpty(), false)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.Padding(3)
				[
					SNew(STextBlock)
					.Text(this, &FSlateVectorShapeDataDetails::GetVerticesNumText)
					.Font(IDetailLayoutBuilder::GetDetailFontBold())
				]
			+SVerticalBox::Slot()
			.Padding(3)
				[
					SNew(STextBlock)
					.Text(this, &FSlateVectorShapeDataDetails::GetTrianglesNumText)
					.Font(IDetailLayoutBuilder::GetDetailFontBold())
				]
			+SVerticalBox::Slot()
			.Padding(3)
				[
					SNew(STextBlock)
					.Text(this, &FSlateVectorShapeDataDetails::GetIndicesNumText)
					.Font(IDetailLayoutBuilder::GetDetailFontBold())
				]
			+SVerticalBox::Slot()
			.Padding(3)
				[
					SNew(STextBlock)
					.Text(this, &FSlateVectorShapeDataDetails::GetVectorShapeSizeText)
					.Font(IDetailLayoutBuilder::GetDetailFontBold())
				]
		];



	SlateVectorDataCategory.AddCustomRow(FText::GetEmpty(), false)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(8)
				[
					SNew(SScaleBox)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
						[
							SNew(SBorder)
							.BorderImage(FVectorShapeEditorStyle::Get()->GetBrush("Checker"))	
								[							
									SAssignNew(VectorShapeMeshWidget, SVectorShapeWidget)
									.Clipping(EWidgetClipping::ClipToBounds)										
								]
						]
				]
		];


	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailBuilder.GetSelectedObjects();
	if (SelectedObjects.Num() > 0)
	{
		USlateVectorShapeData* VectorDataAsset = Cast<USlateVectorShapeData>(SelectedObjects[SelectedObjects.Num() - 1].Get());
		if (VectorDataAsset != nullptr)
		{
			VectorShapeMeshWidget->InitRenderData(*VectorDataAsset , nullptr /* Default Material */);
			SelectedVectorShapeData = VectorDataAsset;
		}
	}
}


FText FSlateVectorShapeDataDetails::GetVerticesNumText() const
{
	FText VerticesNumText = LOCTEXT("InvalidAsset", "InvalidAsset");

	if (SelectedVectorShapeData.IsValid())
	{
		const TArray<FSlateVectorMeshVertex>& Vertices = SelectedVectorShapeData->GetVertexData();
		const int32 VerticesNum = Vertices.Num();

		FNumberFormattingOptions NoCommas;
		NoCommas.UseGrouping = false;

		VerticesNumText = FText::Format(LOCTEXT("VerticesNum", "Vertices : {0}"), FText::AsNumber(VerticesNum, &NoCommas));
	}

	return  VerticesNumText;
}

FText FSlateVectorShapeDataDetails::GetIndicesNumText() const
{
	FText IndicesNumText = LOCTEXT("InvalidAsset", "InvalidAsset");

	if (SelectedVectorShapeData.IsValid())
	{
		const TArray<uint32>& Indices = SelectedVectorShapeData->GetIndexData();
		const int32 IndicesNum = Indices.Num();

		FNumberFormattingOptions NoCommas;
		NoCommas.UseGrouping = false;

		IndicesNumText = FText::Format(LOCTEXT("IndicesNum", "Indices : {0}"), FText::AsNumber(IndicesNum, &NoCommas));
	}

	return  IndicesNumText;
}

FText FSlateVectorShapeDataDetails::GetTrianglesNumText() const
{
	FText TrianglesNumText = LOCTEXT("InvalidAsset", "InvalidAsset");

	if (SelectedVectorShapeData.IsValid())
	{
		const TArray<uint32>& Indices = SelectedVectorShapeData->GetIndexData();
		const int32 TrianglesNum = Indices.Num() / 3;

		FNumberFormattingOptions NoCommas;
		NoCommas.UseGrouping = false;

		TrianglesNumText = FText::Format(LOCTEXT("TrianglesNum", "Triangles : {0}"), FText::AsNumber(TrianglesNum, &NoCommas));
	}

	return  TrianglesNumText;
}

FText FSlateVectorShapeDataDetails::GetVectorShapeSizeText() const
{
	FText VectorShapeSizeText = LOCTEXT("InvalidAsset", "InvalidAsset");
	if (SelectedVectorShapeData.IsValid())
	{
		const FString WorldSizeStr = TEXT("World Size : ") + SelectedVectorShapeData->GetMeshSize().ToString();
		VectorShapeSizeText = FText::FromString(WorldSizeStr);
	}

	return VectorShapeSizeText;
}

void FSlateVectorShapeDataDetails::RefreshDetails(USlateVectorShapeData* SlateVectorShapeData)
{
	if (DetailBuilderPtr != nullptr)
	{
		DetailBuilderPtr->ForceRefreshDetails();
	}
}

#undef LOCTEXT_NAMESPACE
