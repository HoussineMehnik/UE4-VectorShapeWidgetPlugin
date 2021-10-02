//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#pragma once

#include "CoreMinimal.h"
#include "VectorShapeTypes.h"
#include "GameFramework/Actor.h"
#include "VectorShapeActor.generated.h"



class UVectorMeshComponent;
class USlateVectorShapeData;

UCLASS(NotBlueprintable , hidecategories = (Activation, "Components|Activation", Input))
class VECTORSHAPE_API AVectorShapeActor : public AActor
{
	GENERATED_UCLASS_BODY()

		UPROPERTY(Category = "Vector Shape", EditAnywhere, BlueprintReadOnly)
		FVector2D WorldSize;

#if WITH_EDITORONLY_DATA

	UPROPERTY(Category = "Vector Shape Slate Data", EditAnywhere)
		USlateVectorShapeData* SlateVectorData;

	/** If true, the spline will be rendered if the Splines showflag is set. */
	UPROPERTY(EditAnywhere, Category = "Vector Spline")
		bool bDrawDebugSplines;

	UPROPERTY(EditAnywhere, Category = "Vector Spline")
		bool bDrawSpawnRect;

	UPROPERTY(EditAnywhere, Category = "Vector Spline")
		FColor SpawnRectColor;

	UPROPERTY(EditAnywhere, Category = "Vector Spline")
		FVector NewSplineSpawnPoint;

	UPROPERTY(EditAnywhere, Category = "Vector Spline")
		FVector2D NewSplineExtent;

	UPROPERTY(EditAnywhere, Category = "Vector Spline")
		FColor SplineDefaultColor;

	UPROPERTY(Category = "Vector Shape Mesh", EditAnywhere)
		bool bDrawMesh;

	UPROPERTY(Category = "Vector Shape Mesh", EditAnywhere, meta = (ClampMin = "1.0", UIMin = "1.0"))
		float LineDefaultWidth;
	
	/** Shape Simplification & Optimization	**/
	UPROPERTY(Category = "Vector Shape Mesh", EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
		float SimplificationThreshold;

	UPROPERTY(Category = "Vector Shape Mesh", EditAnywhere)
		bool bUseFixedStepsPerSegment;

	/** If bFilterDrivenBones is specified, only these bones will be modified by this node */
	UPROPERTY(Category = "Vector Shape Mesh", EditAnywhere, meta = (EditCondition = "bUseFixedStepsPerSegment", ClampMin = "1", UIMin = "1", ClampMax = "100", UIMax = "100"))
		int32 StepsPerSegment;

#endif //WITH_EDITORONLY_DATA


private:

	UPROPERTY()
		UVectorMeshComponent* MeshComponent;

public:

	/** Returns RenderComponent subobject **/
	FORCEINLINE  UVectorMeshComponent* GetMeshComponent() const { return MeshComponent; }

	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void RebuildVectorShapeMesh();
#endif //WITH_EDITOR

private:
	void CheckTransform();

};
