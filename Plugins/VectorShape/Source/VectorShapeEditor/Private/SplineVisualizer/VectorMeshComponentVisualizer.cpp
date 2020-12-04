#include "VectorMeshComponentVisualizer.h"
#include "VectorMeshComponent.h"
#include "VectorShapeActor.h"
#include "SceneManagement.h"
#include "ComponentVisualizer.h"



#define LOCTEXT_NAMESPACE "VectorMeshComponentVisualizer"

FVectorMeshComponentVisualizer::FVectorMeshComponentVisualizer()
	: VectorShapeActorPtr(nullptr)
{

}

FVectorMeshComponentVisualizer::~FVectorMeshComponentVisualizer()
{

}

void FVectorMeshComponentVisualizer::OnRegister()
{

}

void FVectorMeshComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if (const UVectorMeshComponent* SplineComp = Cast<const UVectorMeshComponent>(Component))
	{
		if (AVectorShapeActor* VectorShapeActor = Cast<AVectorShapeActor>(SplineComp->GetOwner()))
		{

			if (!VectorShapeActor->bDrawSpawnRect)
			{
				return;
			}

			const FVector SpawnRectLocation = VectorShapeActor->GetActorTransform().TransformPosition(VectorShapeActor->NewSplineSpawnPoint);
			const FVector2D& RectExtent = VectorShapeActor->NewSplineExtent;
			const FVector ActorLocation = VectorShapeActor->GetActorLocation();
			const FVector2D& WorldExtent = VectorShapeActor->WorldSize / 2.0f;
			const FVector ActorRight = VectorShapeActor->GetActorRightVector();
			const FVector ActorForward = VectorShapeActor->GetActorForwardVector();
			const FColor& RectColor = VectorShapeActor->SpawnRectColor;

			if (SplineComp == VectorShapeActor->GetMeshComponent())
			{
				PDI->SetHitProxy(new HComponentVisProxy(Component));
			}

			PDI->DrawLine(SpawnRectLocation - ActorForward* RectExtent.X + ActorRight * RectExtent.Y, SpawnRectLocation + ActorForward*RectExtent.X + ActorRight *RectExtent.Y, RectColor, SDPG_Foreground);
			PDI->DrawLine(SpawnRectLocation + ActorForward*RectExtent.X + ActorRight *RectExtent.Y, SpawnRectLocation + ActorForward*RectExtent.X - ActorRight *RectExtent.Y, RectColor, SDPG_Foreground);
			PDI->DrawLine(SpawnRectLocation + ActorForward* RectExtent.X - ActorRight *RectExtent.Y, SpawnRectLocation - ActorForward*RectExtent.X - ActorRight *RectExtent.Y, RectColor, SDPG_Foreground);
			PDI->DrawLine(SpawnRectLocation - ActorForward*RectExtent.X - ActorRight *RectExtent.Y, SpawnRectLocation - ActorForward*RectExtent.X + ActorRight *RectExtent.Y, RectColor, SDPG_Foreground);

			PDI->DrawLine(SpawnRectLocation - ActorForward* RectExtent.X, SpawnRectLocation + ActorForward * RectExtent.X, RectColor, SDPG_Foreground);
			PDI->DrawLine(SpawnRectLocation - ActorRight * RectExtent.Y, SpawnRectLocation + ActorRight *RectExtent.Y, RectColor, SDPG_Foreground);

			PDI->DrawPoint(SpawnRectLocation - ActorForward* RectExtent.X + ActorRight * RectExtent.Y, RectColor, 10.0f, SDPG_Foreground);
			PDI->DrawPoint(SpawnRectLocation + ActorForward*RectExtent.X + ActorRight *RectExtent.Y, RectColor, 10.0f, SDPG_Foreground);
			PDI->DrawPoint(SpawnRectLocation + ActorForward* RectExtent.X - ActorRight *RectExtent.Y, RectColor, 10.0f, SDPG_Foreground);
			PDI->DrawPoint(SpawnRectLocation - ActorForward*RectExtent.X - ActorRight *RectExtent.Y, RectColor, 10.0f, SDPG_Foreground);



			PDI->SetHitProxy(nullptr);
		}
	}
}

bool FVectorMeshComponentVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	if (VisProxy && VisProxy->Component.IsValid())
	{
		if(const UVectorMeshComponent* SplineComp = CastChecked<const UVectorMeshComponent>(VisProxy->Component.Get()))
		{
			if (AVectorShapeActor* VectorShapeActor = Cast<AVectorShapeActor>(SplineComp->GetOwner()))
			{
				VectorShapeActorPtr = VectorShapeActor;

				return true;
			}
		}
	}

	return false;
}

void FVectorMeshComponentVisualizer::EndEditing()
{
	VectorShapeActorPtr = nullptr;
}

bool FVectorMeshComponentVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	if (AVectorShapeActor* VectorShapeActor = VectorShapeActorPtr.Get())
	{
		OutLocation = VectorShapeActor->GetActorTransform().TransformPosition(VectorShapeActor->NewSplineSpawnPoint );
		return true;
	}

	return false;
}

bool FVectorMeshComponentVisualizer::GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const
{
	if (AVectorShapeActor* VectorShapeActor = VectorShapeActorPtr.Get())
	{
		OutMatrix = FRotationMatrix::Make(VectorShapeActor->GetActorQuat());
		return true;
	}
	return false;
}

bool FVectorMeshComponentVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	if (AVectorShapeActor* VectorShapeActor = VectorShapeActorPtr.Get())
	{

		VectorShapeActor->Modify();

		if (!DeltaTranslate.IsZero())
		{
			VectorShapeActor->NewSplineSpawnPoint += VectorShapeActor->GetActorTransform().InverseTransformVector(DeltaTranslate);
			VectorShapeActor->NewSplineSpawnPoint.Z = FMath::Max<float>(VectorShapeActor->NewSplineSpawnPoint.Z, 0.0f);
		}

		if (!DeltaScale.IsZero())
		{
			VectorShapeActor->NewSplineExtent += FVector2D(DeltaScale.X, DeltaScale.Y) * 100.0f;
			VectorShapeActor->NewSplineExtent.X = FMath::Max<float>(VectorShapeActor->NewSplineExtent.X, 5.0f);
			VectorShapeActor->NewSplineExtent.Y = FMath::Max<float>(VectorShapeActor->NewSplineExtent.Y, 5.0f);
		}

		if (!DeltaRotate.IsZero())
		{

		}

		return true;
	}

	return false;
}

bool FVectorMeshComponentVisualizer::HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{


	return false;
}


#undef LOCTEXT_NAMESPACE
