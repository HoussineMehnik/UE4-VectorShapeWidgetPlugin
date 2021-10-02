//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#include "VectorShapeEditorHelpers.h"
#include "VectorSplineComponent.h"
#include "VectorShapeActor.h"
#include "ScopedTransaction.h"

#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"

#include "Kismet2/ComponentEditorUtils.h"

#include "VectorMeshComponent.h"
#include "VectorShapeEditorTypes.h"

#include "VectorShapeTypes.h"

#include "PolygonTools/VectorPolygonTools.h"


#define LOCTEXT_NAMESPACE "VectorShapeEditorHelpers"

UVectorSplineComponent* VectorShapeEditorHelpers::AddNewSplineComponent(AVectorShapeActor* VectorShapeActor, bool bIsPolygonShape)
{
	if (VectorShapeActor == nullptr || VectorShapeActor->IsPendingKillOrUnreachable())
	{
		return nullptr;
	}

	// Invalidate any active component in the visualizer
	GUnrealEd->ComponentVisManager.ClearActiveComponentVis();

	GUnrealEd->SelectNone(false, true);

	UVectorSplineComponent* NewComponent = nullptr;

	const FScopedTransaction Transaction(LOCTEXT("AddVectorSplineComponent", "Add Vector Spline Component"));

	{
		// No template, so create a wholly new component
		VectorShapeActor->Modify();

		// Create an appropriate name for the new component
		FName NewComponentName = *FComponentEditorUtils::GenerateValidVariableName(UVectorSplineComponent::StaticClass(), VectorShapeActor);

		// Get the set of owned components that exists prior to instancing the new component.
		TInlineComponentArray<UActorComponent*> PreInstanceComponents;
		VectorShapeActor->GetComponents(PreInstanceComponents);

		// Construct the new component and attach as needed
		UActorComponent* NewInstanceComponent = NewObject<UActorComponent>(VectorShapeActor, UVectorSplineComponent::StaticClass(), NewComponentName, RF_Transactional);
		if (USceneComponent* NewSceneComponent = Cast<USceneComponent>(NewInstanceComponent))
		{
			USceneComponent* RootComponent = VectorShapeActor->GetRootComponent();
			if (RootComponent)
			{
				NewSceneComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			}
			else
			{
				VectorShapeActor->SetRootComponent(NewSceneComponent);
			}
		}

		// Add to SerializedComponents array so it gets saved
		VectorShapeActor->AddInstanceComponent(NewInstanceComponent);
		NewInstanceComponent->OnComponentCreated();
		NewInstanceComponent->RegisterComponent();

		// Register any new components that may have been created during construction of the instanced component, but were not explicitly registered.
		TInlineComponentArray<UActorComponent*> PostInstanceComponents;
		VectorShapeActor->GetComponents(PostInstanceComponents);
		for (UActorComponent* ActorComponent : PostInstanceComponents)
		{
			if (!ActorComponent->IsRegistered() && ActorComponent->bAutoRegister && !ActorComponent->IsPendingKill() && !PreInstanceComponents.Contains(ActorComponent))
			{
				ActorComponent->RegisterComponent();
			}
		}

		// Rerun construction scripts
		VectorShapeActor->RerunConstructionScripts();

		NewComponent = Cast<UVectorSplineComponent>(NewInstanceComponent);

		if (NewComponent != nullptr)
		{

			NewComponent->InitSpline(
				VectorShapeActor->NewSplineSpawnPoint,
				VectorShapeActor->NewSplineExtent,
				VectorShapeActor->SplineDefaultColor,
				bIsPolygonShape ? EVectorSplineType::Polygon : EVectorSplineType::Line, 
				VectorShapeActor->bDrawDebugSplines
			);

			NewComponent->MarkRenderStateDirty();
		}

	}


	GUnrealEd->SelectActor(VectorShapeActor, true, true, true, true);
	GUnrealEd->RedrawLevelEditingViewports(true);

	return NewComponent;
}

UVectorSplineComponent* VectorShapeEditorHelpers::DuplicateSplineComponent(UVectorSplineComponent* SplineComponent)
{
	

	if (SplineComponent == nullptr || SplineComponent->IsPendingKillOrUnreachable())
	{
		return nullptr;
	}

	AActor* VectorShapeActor = SplineComponent->GetOwner();

	if (VectorShapeActor == nullptr || VectorShapeActor->IsPendingKillOrUnreachable())
	{
		return nullptr;;
	}


	// Invalidate any active component in the visualizer
	GUnrealEd->ComponentVisManager.ClearActiveComponentVis();

	GUnrealEd->SelectNone(false, true);

	UVectorSplineComponent* CloneComponent = nullptr;

	const FScopedTransaction Transaction(LOCTEXT("DuplicateSplineComponent", "Duplicate Vector Spline Component"));

	{
		UActorComponent* NewInstanceComponent = FComponentEditorUtils::DuplicateComponent(SplineComponent);

		CloneComponent = Cast<UVectorSplineComponent>(NewInstanceComponent);
		if (CloneComponent != nullptr)
		{
			CloneComponent->CheckSpline();
			CloneComponent->MarkRenderStateDirty();
		}

	}


	GUnrealEd->SelectActor(VectorShapeActor, true, true, true, true);
	GUnrealEd->RedrawLevelEditingViewports(true);

	return CloneComponent;

}

void VectorShapeEditorHelpers::RemoveSplineComponent(UVectorSplineComponent* SplineComponent)
{
	if (SplineComponent == nullptr || SplineComponent->IsPendingKillOrUnreachable())
	{
		return;
	}

	AActor* VectorShapeActor = SplineComponent->GetOwner();

	if (VectorShapeActor == nullptr || VectorShapeActor->IsPendingKillOrUnreachable())
	{
		return;
	}


	// Invalidate any active component in the visualizer
	GUnrealEd->ComponentVisManager.ClearActiveComponentVis();
	const FScopedTransaction Transaction(LOCTEXT("RemoveVectorSplineComponent", "Remove vector Spline Component"));

	VectorShapeActor->Modify(true);

	TArray<UActorComponent*> ComponentsToDelete;
	ComponentsToDelete.Add(SplineComponent);

	UActorComponent* ComponentToSelect = nullptr;

	FComponentEditorUtils::DeleteComponents(ComponentsToDelete, ComponentToSelect);
}

void VectorShapeEditorHelpers::ClearAllSplineComponents(AVectorShapeActor* VectorShapeActor)
{
		if (VectorShapeActor == nullptr || VectorShapeActor->IsPendingKillOrUnreachable())
	{
		return;
	}


	// Invalidate any active component in the visualizer
	GUnrealEd->ComponentVisManager.ClearActiveComponentVis();
	const FScopedTransaction Transaction(LOCTEXT("RemoveVectorSplineComponent", "Remove all vector Spline Components"));

	VectorShapeActor->Modify(true);

	TArray<UActorComponent*> SplineComps;
	VectorShapeActor->GetComponents(UVectorSplineComponent::StaticClass(), SplineComps);

	UActorComponent* ComponentToSelect = nullptr;

	FComponentEditorUtils::DeleteComponents(SplineComps, ComponentToSelect);
}

// Dont forget Transaction when you add multi comps
void VectorShapeEditorHelpers::OnCreateVectorMesh(AVectorShapeActor* VectorShapeActor)
{
	if (VectorShapeActor == nullptr || VectorShapeActor->IsPendingKill())
	{
		return;
	}

	UVectorMeshComponent* VectorMeshComp = VectorShapeActor->GetMeshComponent();
	{
		VectorMeshComp->ClearMeshSections(false);
	}


	if (VectorShapeActor->bDrawMesh)
	{
		// Generate Polygon
		TArray<FVectorShapePolygon> SplinesSimplifiedPolygons;

		{
			TArray<UActorComponent*> SplineComps;
			VectorShapeActor->GetComponents(UVectorSplineComponent::StaticClass(), SplineComps);
			SplinesSimplifiedPolygons.Reset(SplineComps.Num() * 4);
			for (auto* SplineComp : SplineComps)
			{
				if (UVectorSplineComponent* Spline2DComponent = Cast<UVectorSplineComponent>(SplineComp))
				{

					if (!Spline2DComponent->bDrawMesh)
					{
						continue;
					}

					{
						Spline2DComponent->CheckSpline();
					}

					{
						FVectorShapePolygon& NewPolygon = *new (SplinesSimplifiedPolygons) FVectorShapePolygon(Spline2DComponent);
						FPolygonTools::SimplifySplinePolygon(NewPolygon);
						FPolygonTools::RemoveCollinearPoints(NewPolygon.Vertices2D);
						FPolygonTools::RemoveDuplicatedPoints(NewPolygon.Vertices2D);
					}
				}
			}
		}

		//
		{
			const int32 NumPolygons = SplinesSimplifiedPolygons.Num();
			for (int32 a = 0; a < NumPolygons; a++)
			{
				for (int32 b = 0; b < NumPolygons; b++)
				{
					const float& PolyADepth = SplinesSimplifiedPolygons[a].SplineComponent->OffsetZ;
					const float& PolyBDepth = SplinesSimplifiedPolygons[b].SplineComponent->OffsetZ;
					if (PolyADepth < PolyBDepth)
					{
						// Swap the order in which polygons will be drawn
						SplinesSimplifiedPolygons.Swap(a, b);
					}
				}
			}
		}


		{
			const float& LineWidth = VectorShapeActor->LineDefaultWidth;
			const FVector2D& WorldSize = VectorShapeActor->WorldSize;

			VectorMeshComp->InitMeshSections(1);
			FVectorShapeMeshSection* MeshSection = VectorMeshComp->GetVectorMeshSection(0);


			int32 RunningIndex = 0;

			FVector2D Norm(ForceInitToZero);

			for (FVectorShapePolygon Polygon : SplinesSimplifiedPolygons)
			{
				const FColor& VertexColor = Polygon.SplineComponent->SplineColor;
				const float& OffsetZ = Polygon.SplineComponent->OffsetZ;

				if (Polygon.SplineComponent->SplineType == EVectorSplineType::Line)
				{
					const int32 NumVerts = Polygon.Vertices2D.Num();
					for (int32 VertIdx = 0; VertIdx < NumVerts; VertIdx++)
					{
						const FVector2D& P1 = Polygon.Vertices2D[VertIdx];
						const FVector2D& P2 = (VertIdx == NumVerts - 1) ? Polygon.Vertices2D[VertIdx - 1] : Polygon.Vertices2D[VertIdx + 1];

						const FVector2D Dir = (VertIdx == NumVerts - 1) ? (P1 - P2) : (P2 - P1);
						

						 if (VertIdx > 0 && VertIdx < NumVerts - 1)
						 {
							 const FVector2D& P0 = Polygon.Vertices2D[VertIdx - 1];
							 const FVector2D Dir2 = (P1 - P0);
							 Norm = ((FVector2D(Dir.Y, -Dir.X).GetSafeNormal() + FVector2D(Dir2.Y, -Dir2.X).GetSafeNormal()) / 2.0f).GetSafeNormal();
						 }
						 else
						 {
							  Norm = FVector2D(Dir.Y, -Dir.X).GetSafeNormal();
						 }

						const float S0 = Polygon.SplineComponent->FindScaleClosestToWorldLocation(VectorShapeActor->GetActorTransform().TransformPosition(FVector(P1, 0))).Y;

						const FVector V0 = FVector(P1 + Norm * LineWidth * S0, OffsetZ);
						const FVector V1 = FVector(P1 - Norm * LineWidth * S0, OffsetZ);

						MeshSection->Vertices.Add(V0);
						MeshSection->Vertices.Add(V1);

						MeshSection->VertexColors.Add(VertexColor);
						MeshSection->VertexColors.Add(VertexColor);

						MeshSection->TextureCoordinates.Add(FVector2D(V0.X, V0.Y) / WorldSize);
						MeshSection->TextureCoordinates.Add(FVector2D(V1.X, V1.Y) / WorldSize);

						MeshSection->SectionLocalBox += V0;
						MeshSection->SectionLocalBox += V1;

						RunningIndex += 2;

						if (VertIdx < NumVerts - 1)
						{
							const int32 T0 = RunningIndex - 2;
							const int32 T1 = RunningIndex - 1;
							const int32 T2 = T0 + 2;
							const int32 T3 = T0 + 3;

							MeshSection->Indices.Add(T0);
							MeshSection->Indices.Add(T1);
							MeshSection->Indices.Add(T3);

							MeshSection->Indices.Add(T3);
							MeshSection->Indices.Add(T2);
							MeshSection->Indices.Add(T0);
						}
					}
				}
				else
				{

					FPolygonTools::TriangulatePolygon(Polygon.Vertices2D, MeshSection->Indices, RunningIndex, false);



					for (const FVector2D& Vertex2D : Polygon.Vertices2D)
					{
						const FVector Vertex3D = FVector(Vertex2D.X, Vertex2D.Y, OffsetZ);
						MeshSection->Vertices.Add(Vertex3D);
						MeshSection->VertexColors.Add(VertexColor);
						MeshSection->TextureCoordinates.Add(Vertex2D / WorldSize);
						MeshSection->SectionLocalBox += Vertex3D;
					}

					RunningIndex += Polygon.Vertices2D.Num();
				}
			}


			if (RunningIndex != 0)
			{
				MeshSection->Tangents.Add(FVector::ForwardVector);
				MeshSection->Normals.Add(FVector::UpVector);

				if (MeshSection->TextureCoordinates.Num() == 0)
					MeshSection->TextureCoordinates.Add(FVector2D(1.f, 1.f));

			}
			VectorMeshComp->SetMaterial(0, FVectorShapeRuntimeHelpers::GetVectorMeshDefaultMaterial());
		}
	}

	// Finalize Mesh
	VectorMeshComp->MarkMeshDirty();
}




#undef LOCTEXT_NAMESPACE
