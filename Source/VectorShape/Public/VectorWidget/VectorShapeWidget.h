//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#pragma once

#include "Components/Widget.h"
#include "VectorShapeTypes.h"
#include "VectorShapeWidget.generated.h"



class USlateVectorShapeData;
class SVectorShapeWidget;
class UMaterialInterface;
class UTexture;

UCLASS(ClassGroup = Custom)
class VECTORSHAPE_API UVectorShapeWidget : public UWidget
{
	GENERATED_BODY()

public:
	UVectorShapeWidget();

	//~ Begin UWidget Interface
	virtual void SynchronizeProperties() override;
	//~ End UWidget Interface

	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

#if WITH_EDITOR
	//~ Begin UWidget Interface
	virtual const FText GetPaletteCategory() override;
	//~ End UWidget Interface
#endif

protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~ End UWidget Interface

protected:
	UPROPERTY(EditAnywhere, Category = "Appearance")
		USlateVectorShapeData* VectorDataAsset;

	UPROPERTY(EditAnywhere, Category = "Appearance", meta = (InlineEditConditionToggle))
		bool bOverrideMaterial;

	UPROPERTY(EditAnywhere, Category = "Appearance", meta = (editcondition = "bOverrideMaterial"))
		UMaterialInterface* MaterialOverride;

private:
	TSharedPtr<SVectorShapeWidget> SlateVectorWidget;


public:

	UFUNCTION(BlueprintCallable, Category = "VectorShapeWidget")
		void UpdateLinesData(const TArray<FVectorLineData>& InLinesData);

	UFUNCTION(BlueprintCallable, Category = "VectorShapeWidget")
		void ClearLinesData();


	UFUNCTION(BlueprintCallable, Category = "VectorShapeWidget")
		void SetDataAsset(USlateVectorShapeData* InDataAsset);

	UFUNCTION(BlueprintCallable, Category = "VectorShapeWidget|Material")
		void SetMaterial(UMaterialInterface* NewMaterial);

	UFUNCTION(BlueprintCallable, Category = "VectorShapeWidget|Material")
		UMaterialInstanceDynamic* GetDynamicMaterial();

	UFUNCTION(BlueprintCallable, Category = "VectorShapeWidget|Material")
		void SetMaterialScalarParameter(FName ParameterName, float InValue);

	UFUNCTION(BlueprintCallable, Category = "VectorShapeWidget|Material")
		void SetMaterialVectorParameter(FName ParameterName, const FLinearColor& InValue);

	UFUNCTION(BlueprintCallable, Category = "VectorShapeWidget|Material")
		void SetTextureParameterValue(FName ParameterName, class UTexture* InValue);
};
