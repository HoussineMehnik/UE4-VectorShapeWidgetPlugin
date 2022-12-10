//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#include "TypeActions/AssetTypeActions_VectorShapeData.h"
#include "Layout/Margin.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "EditorStyleSet.h"
#include "IIntroTutorials.h"
#include "EditorTutorial.h"
#include "AssetRegistry/AssetData.h"
#include "Widgets/SBoxPanel.h"
#include "VectorWidget/SlateVectorShapeData.h"
#include "VectorWidget/SVectorShapeWidget.h"
#include "Widgets/Layout/SScaleBox.h"
#include "VectorShapeEditorStyle.h"

#define LOCTEXT_NAMESPACE "FAssetTypeActions_VectorShapeData"


FText FAssetTypeActions_VectorShapeData::GetName() const
{
	return LOCTEXT("AssetTypeActions_VectorShapeData", "Vector Shape Data");
}

FColor FAssetTypeActions_VectorShapeData::GetTypeColor() const
{
	return FColor(129, 196, 115);
}

UClass* FAssetTypeActions_VectorShapeData::GetSupportedClass() const
{
	return USlateVectorShapeData::StaticClass();
}

uint32 FAssetTypeActions_VectorShapeData::GetCategories()
{
	return  EAssetTypeCategories::UI;
}

#undef LOCTEXT_NAMESPACE
