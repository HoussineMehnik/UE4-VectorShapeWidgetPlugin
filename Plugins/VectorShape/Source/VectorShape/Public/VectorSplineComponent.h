// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "VectorShapeTypes.h"
#include "VectorSplineComponent.generated.h"


class UMaterialInterface;

/**
 * 
 */
UCLASS(ClassGroup = Utility, ShowCategories = (Mobility), HideCategories = (Spline,Editor,Physics, Collision, Lighting, Rendering, Mobile), meta = (BlueprintSpawnableComponent))
class VECTORSHAPE_API UVectorSplineComponent : public USplineComponent
{
	GENERATED_UCLASS_BODY()

public:
	// UObject interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// End of UObject interface

	void InitSpline(const FVector& InLocation, const FVector2D& InExtent, const FColor& InColor, EVectorSplineType InType, bool bDraw);

	void CheckSpline();


	UPROPERTY()
		FColor SplineColor;

	UPROPERTY()
		float OffsetZ;

	UPROPERTY()
		EVectorSplineType SplineType;

	UPROPERTY()
		bool bDrawMesh;


#if WITH_EDITORONLY_DATA
	/** Color of Additive spline component segment in the editor */
	UPROPERTY(EditAnywhere, Category = Visualization)
		FLinearColor AdditiveSplineSegmentColor;

	/** Color of Subtractive spline component segment in the editor */
	UPROPERTY(EditAnywhere, Category = Visualization)
		FLinearColor SubtractiveSplineSegmentColor;

	/** Color of a selected spline component segment in the editor */
	UPROPERTY(EditAnywhere, Category = Visualization)
		FLinearColor SelectionColor;
#endif

};
