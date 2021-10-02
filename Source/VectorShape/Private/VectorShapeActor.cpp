//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#include "VectorShapeActor.h"
#include "VectorMeshComponent.h"


#if WITH_EDITOR
#include "VectorSplineComponent.h"
#endif //WITH_EDITOR

#include "UObject/ConstructorHelpers.h"



AVectorShapeActor::AVectorShapeActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, WorldSize(FVector2D(100,100))
#if WITH_EDITORONLY_DATA
	, SlateVectorData(nullptr)
	, bDrawDebugSplines(true)
	, bDrawSpawnRect(true)
	, SpawnRectColor(FColor::Orange)
	, NewSplineSpawnPoint(0.f, 0.f, 0.f)
	, NewSplineExtent(100.0, 100.0f)
	, SplineDefaultColor(FColor::White)
	, bDrawMesh(true)
	, LineDefaultWidth(100.0f)
	, SimplificationThreshold(0.98f)
	, bUseFixedStepsPerSegment(true)
	, StepsPerSegment(20)
#endif //WITH_EDITOR
{
	MeshComponent = CreateDefaultSubobject<UVectorMeshComponent>(TEXT("MeshComponent0"));
	RootComponent = MeshComponent;
}



void AVectorShapeActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	CheckTransform();
}




#if WITH_EDITOR

void AVectorShapeActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	//
	CheckTransform();

	SimplificationThreshold = FMath::Clamp<float>(SimplificationThreshold, 0.0f, 1.0f);

	//
	NewSplineSpawnPoint.Z = FMath::Max<float>(NewSplineSpawnPoint.Z, 0.0f);

	//
	NewSplineExtent.X = FMath::Max<float>(NewSplineExtent.X, 5.0f);
	NewSplineExtent.Y = FMath::Max<float>(NewSplineExtent.Y, 5.0f);


	//
	WorldSize.X = FMath::Max<float>(WorldSize.X, 5.0f);
	WorldSize.Y = FMath::Max<float>(WorldSize.Y, 5.0f);

	//
	{
		TArray<UActorComponent*> SplineComponents;
		this->GetComponents(UVectorSplineComponent::StaticClass(), SplineComponents);

		for (auto* SplineComp : SplineComponents)
		{
			if (UVectorSplineComponent* VectorSplineComp = Cast<UVectorSplineComponent>(SplineComp))
			{
				//
				VectorSplineComp->SetDrawDebug(bDrawDebugSplines);

				//
				if (!VectorSplineComp->bIsEditorOnly)
				{
					VectorSplineComp->bIsEditorOnly = true;
				}
			}
		}
	}


	RebuildVectorShapeMesh();
}

void AVectorShapeActor::RebuildVectorShapeMesh()
{
	FVectorShapeEditorDelegates::OnCreateVectorMeshDelegate.Broadcast(this);
}

#endif //WITH_EDITOR


void AVectorShapeActor::CheckTransform()
{
	if (!GetActorScale3D().IsUniform())
	{
		SetActorScale3D(FVector::OneVector);
	}

	const FQuat ActorQuat = GetActorQuat();
	if (ActorQuat.X != 0.0f || ActorQuat.Y != 0.0f)
	{
		SetActorRotation(FQuat(0.0f, 0.0f, ActorQuat.Z, ActorQuat.W));
	}
}
