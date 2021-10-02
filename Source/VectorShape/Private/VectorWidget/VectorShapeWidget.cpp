//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#include "VectorWidget/VectorShapeWidget.h"
#include "VectorWidget/SVectorShapeWidget.h"
#include "VectorWidget/SlateVectorShapeData.h"
#include "VectorShapeTypes.h"

#include "Materials/MaterialInstanceDynamic.h"



#define LOCTEXT_NAMESPACE "VectorShapeWidget"

/////////////////////////////////////////////////////
// UVectorShapeWidget

UVectorShapeWidget::UVectorShapeWidget()
	: VectorDataAsset(nullptr)	
	, bOverrideMaterial(false)
	, MaterialOverride(nullptr)
{

}

void UVectorShapeWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (SlateVectorWidget.IsValid())
	{
		if (MaterialOverride != nullptr && MaterialOverride->IsPendingKill())
		{
			MaterialOverride = nullptr;
		}

		if (VectorDataAsset == nullptr || VectorDataAsset->IsPendingKill())
		{
			SlateVectorWidget->ClearRenderData();
		}
		else
		{
			SlateVectorWidget->InitRenderData(*VectorDataAsset, MaterialOverride);
		}
	}
}

void UVectorShapeWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	SlateVectorWidget.Reset();
}

#if WITH_EDITOR

const FText UVectorShapeWidget::GetPaletteCategory()
{
	return LOCTEXT("Custom", "Custom");
}

#endif

TSharedRef<SWidget> UVectorShapeWidget::RebuildWidget()
{
	SlateVectorWidget = SNew(SVectorShapeWidget);
	return SlateVectorWidget.ToSharedRef();
}


void UVectorShapeWidget::UpdateLinesData(const TArray<FVectorLineData>& InLinesData)
{
	if (SlateVectorWidget.IsValid())
	{
		SlateVectorWidget->UpdateLinesData(InLinesData);
	}
}

void UVectorShapeWidget::ClearLinesData()
{
	if (SlateVectorWidget.IsValid())
	{
		SlateVectorWidget->ClearLinesData();
	}
}

void UVectorShapeWidget::SetDataAsset( USlateVectorShapeData* InDataAsset)
{
	if (SlateVectorWidget.IsValid())
	{
		if (InDataAsset == nullptr)
		{
			VectorDataAsset = nullptr;
			SlateVectorWidget->ClearRenderData();
		}
		else
		{
			VectorDataAsset = InDataAsset;
			SlateVectorWidget->InitRenderData(*VectorDataAsset, MaterialOverride);
		}
	}
}

void UVectorShapeWidget::SetMaterial(UMaterialInterface* NewMaterial)
{
	if (NewMaterial != nullptr)
	{
		MaterialOverride = NewMaterial;
		if (SlateVectorWidget.IsValid())
		{
			SlateVectorWidget->SetBrushMaterial(MaterialOverride);
		}
	}
}

UMaterialInstanceDynamic* UVectorShapeWidget::GetDynamicMaterial()
{
	if (SlateVectorWidget.IsValid())
	{
		return SlateVectorWidget->GetMaterialInstanceDynamicSafe();
	}
	return nullptr;
}

void UVectorShapeWidget::SetMaterialScalarParameter(FName ParameterName, float InValue)
{
	if (UMaterialInstanceDynamic* MatInstDynamic = GetDynamicMaterial())
	{
		MatInstDynamic->SetScalarParameterValue(ParameterName, InValue);
	}
}


void UVectorShapeWidget::SetMaterialVectorParameter(FName ParameterName, const FLinearColor& InValue)
{
	if (UMaterialInstanceDynamic* MatInstDynamic = GetDynamicMaterial())
	{
		MatInstDynamic->SetVectorParameterValue(ParameterName, InValue);
	}
}


void UVectorShapeWidget::SetTextureParameterValue(FName ParameterName, class UTexture* InValue)
{
	if (UMaterialInstanceDynamic* MatInstDynamic = GetDynamicMaterial())
	{
		MatInstDynamic->SetTextureParameterValue(ParameterName, InValue);
	}
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE