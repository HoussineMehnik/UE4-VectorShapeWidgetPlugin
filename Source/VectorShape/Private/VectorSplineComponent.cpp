//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#include "VectorSplineComponent.h"
#include "VectorShapeActor.h"



UVectorSplineComponent::UVectorSplineComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SplineColor(FColor::White)
	, OffsetZ(0.0f)
	, SplineType(EVectorSplineType::Polygon)
	, bDrawMesh(true)
#if WITH_EDITORONLY_DATA
	, AdditiveSplineSegmentColor(FLinearColor(0.25f, 0.25f, 1.0f))
	, SubtractiveSplineSegmentColor(FLinearColor(1.0f, 0.25f, 0.25f))
	, SelectionColor(FLinearColor(1.0f, 0.0f, 0.0f))
#endif

{


}



#if WITH_EDITOR
void UVectorSplineComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	CheckSpline();

	if (AVectorShapeActor* SplineOwner = Cast<AVectorShapeActor>(GetOwner()))
	{
		SplineOwner->RebuildVectorShapeMesh();
	}
}
#endif //WITH_EDITOR


void UVectorSplineComponent::InitSpline(const FVector& InLocation, const FVector2D& InExtent, const FColor& InColor, EVectorSplineType InType, bool bDraw)
{
	SplineCurves.Position.Points.Reset(10);
	SplineCurves.Rotation.Points.Reset(10);
	SplineCurves.Scale.Points.Reset(10);
	UpdateSpline();

	SplineCurves.Position.Points.Emplace(0.0f, InLocation + FVector(-InExtent.X, InExtent.Y, 0.0f), FVector::ZeroVector, FVector::ZeroVector, CIM_Linear);
	SplineCurves.Rotation.Points.Emplace(0.0f, FQuat::Identity, FQuat::Identity, FQuat::Identity, CIM_Linear);
	SplineCurves.Scale.Points.Emplace(0.0f, FVector(1.0f), FVector::ZeroVector, FVector::ZeroVector, CIM_Linear);

	SplineCurves.Position.Points.Emplace(1.0f, InLocation + FVector(InExtent.X, InExtent.Y, 0.0f), FVector::ZeroVector, FVector::ZeroVector, CIM_Linear);
	SplineCurves.Rotation.Points.Emplace(1.0f, FQuat::Identity, FQuat::Identity, FQuat::Identity, CIM_Linear);
	SplineCurves.Scale.Points.Emplace(1.0f, FVector(1.0f), FVector::ZeroVector, FVector::ZeroVector, CIM_Linear);


	if (InType == EVectorSplineType::Polygon)
	{
		SplineCurves.Position.Points.Emplace(2.0f, InLocation + FVector(InExtent.X, -InExtent.Y, 0.0f), FVector::ZeroVector, FVector::ZeroVector, CIM_Linear);
		SplineCurves.Rotation.Points.Emplace(2.0f, FQuat::Identity, FQuat::Identity, FQuat::Identity, CIM_Linear);
		SplineCurves.Scale.Points.Emplace(2.0f, FVector(1.0f), FVector::ZeroVector, FVector::ZeroVector, CIM_Linear);

		SplineCurves.Position.Points.Emplace(3.0f, InLocation + FVector(-InExtent.X, -InExtent.Y, 0.0f), FVector::ZeroVector, FVector::ZeroVector, CIM_Linear);
		SplineCurves.Rotation.Points.Emplace(3.0f, FQuat::Identity, FQuat::Identity, FQuat::Identity, CIM_Linear);
		SplineCurves.Scale.Points.Emplace(3.0f, FVector(1.0f), FVector::ZeroVector, FVector::ZeroVector, CIM_Linear);
	}

	SplineType = InType;

	bDrawDebug = bDraw;

	CheckSpline();
}

void UVectorSplineComponent::CheckSpline()
{
	//
	OffsetZ = FMath::Max<float>(OffsetZ, 0.0f);

	//
	const bool bForceClosed = (SplineType == EVectorSplineType::Polygon);
	SetClosedLoop(bForceClosed, true);

	// Check Transform
	if (!GetRelativeLocation().IsNearlyZero() || !GetRelativeRotation().IsNearlyZero() || !GetRelativeScale3D().IsNearlyZero())
	{
		SetRelativeTransform(FTransform::Identity);
	}
}
