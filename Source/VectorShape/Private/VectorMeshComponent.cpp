//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#include "VectorMeshComponent.h"
#include "PrimitiveViewRelevance.h"
#include "RenderResource.h"
#include "RenderingThread.h"
#include "PrimitiveSceneProxy.h"
#include "Containers/ResourceArray.h"
#include "EngineGlobals.h"
#include "VertexFactory.h"
#include "MaterialShared.h"
#include "Materials/Material.h"
#include "LocalVertexFactory.h"
#include "Engine/Engine.h"
#include "SceneManagement.h"
#include "DynamicMeshBuilder.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "StaticMeshResources.h"
#include "VectorShapeTypes.h"
#include "VectorShapeActor.h"
#include "SceneInterface.h"
#include "MaterialDomain.h"
#include "Materials/MaterialRenderProxy.h"

DECLARE_CYCLE_STAT(TEXT("Create Vector Mesh Proxy"), STAT_VectorMesh_CreateSceneProxy, STATGROUP_VectorMesh);
DECLARE_CYCLE_STAT(TEXT("Get Vector Mesh Elements"), STAT_VectorMesh_GetMeshElements, STATGROUP_VectorMesh);

/** Class representing a single section of the VectorMesh */
class FVectorMeshProxySection
{
public:
	/** Material applied to this section */
	UMaterialInterface* Material;
	/** Vertex buffer for this section */
	FStaticMeshVertexBuffers VertexBuffers;
	/** Index buffer for this section */
	FDynamicMeshIndexBuffer32 IndexBuffer;
	/** Vertex factory for this section */
	FLocalVertexFactory VertexFactory;

	FVectorMeshProxySection(ERHIFeatureLevel::Type InFeatureLevel)
	: Material(NULL)
	, VertexFactory(InFeatureLevel, "FVectorMeshProxySection")
	{}
};

/** Vector Mesh scene proxy */
class FVectorMeshSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FVectorMeshSceneProxy(UVectorMeshComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, MaterialRelevance(InComponent->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	{

		if (AVectorShapeActor* ShapeActor = Cast<AVectorShapeActor>(InComponent->GetOwner()))
		{
			WorldSize = ShapeActor->WorldSize;
		}

		SetWireframeColor(InComponent->GetWireframeColor());


		// Copy each section
		const int32 NumSections = InComponent->VectorMeshSections.Num();
		Sections.AddZeroed(NumSections);
		for (int SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
		{

			FVectorShapeMeshSection& SrcSection = InComponent->VectorMeshSections[SectionIdx];
			if (SrcSection.IsValid())
			{
				FVectorMeshProxySection* NewSection = new FVectorMeshProxySection(GetScene().GetFeatureLevel());

				// Copy data from vertex buffer
				const int32 NumVerts = SrcSection.Vertices.Num();

				// Allocate verts

				TArray<FDynamicMeshVertex> Vertices;
				Vertices.SetNumUninitialized(NumVerts);

				const bool bHasUniqueTangent = (SrcSection.Tangents.Num() < SrcSection.Vertices.Num());
				const bool bHasUniqueNormal = (SrcSection.Normals.Num() < SrcSection.Vertices.Num());
				const bool bHasUniqueUVs = (SrcSection.TextureCoordinates.Num() < SrcSection.Vertices.Num());

				// Copy verts
				for (int VertIdx = 0; VertIdx < NumVerts; VertIdx++)
				{
					FDynamicMeshVertex& Vert = Vertices[VertIdx];
					Vert.Position = (FVector3f)SrcSection.Vertices[VertIdx];
					Vert.Color = (VertIdx < SrcSection.VertexColors.Num()) ? SrcSection.VertexColors[VertIdx] : FColor::White;
					Vert.TextureCoordinate[0] = FVector2f(bHasUniqueUVs ? SrcSection.TextureCoordinates[0] : SrcSection.TextureCoordinates[VertIdx]);
					Vert.TangentX = bHasUniqueTangent ? SrcSection.Tangents[0] : SrcSection.Tangents[VertIdx];
					Vert.TangentZ = bHasUniqueNormal ? SrcSection.Normals[0] : SrcSection.Normals[VertIdx];
					Vert.TangentZ.Vector.W = 127;
				}

				// Copy index buffer
				NewSection->IndexBuffer.Indices = SrcSection.Indices;

				NewSection->VertexBuffers.InitFromDynamicVertex(&NewSection->VertexFactory, Vertices, 4);

				// Enqueue initialization of render resource
				BeginInitResource(&NewSection->VertexBuffers.PositionVertexBuffer);
				BeginInitResource(&NewSection->VertexBuffers.StaticMeshVertexBuffer);
				BeginInitResource(&NewSection->VertexBuffers.ColorVertexBuffer);
				BeginInitResource(&NewSection->IndexBuffer);
				BeginInitResource(&NewSection->VertexFactory);

				// Grab material
				NewSection->Material = InComponent->GetMaterial(SectionIdx);
				if (NewSection->Material == NULL)
				{
					NewSection->Material = UMaterial::GetDefaultMaterial(MD_Surface);
				}

				// Save ref to new section
				Sections[SectionIdx] = NewSection;
			}
		}
	}

	virtual ~FVectorMeshSceneProxy()
	{
		for (FVectorMeshProxySection* Section : Sections)
		{
			if (Section != nullptr)
			{
				Section->VertexBuffers.PositionVertexBuffer.ReleaseResource();
				Section->VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
				Section->VertexBuffers.ColorVertexBuffer.ReleaseResource();
				Section->IndexBuffer.ReleaseResource();
				Section->VertexFactory.ReleaseResource();
				delete Section;
			}
		}
	}


	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		SCOPE_CYCLE_COUNTER(STAT_VectorMesh_GetMeshElements);

		// Set up wireframe material (if needed)
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FColoredMaterialRenderProxy* WireframeMaterialInstance = NULL;
		if (bWireframe)
		{
			WireframeMaterialInstance = new FColoredMaterialRenderProxy(
				GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
				FLinearColor(0, 0.5f, 1.f)
			);

			Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
		}

		const FEngineShowFlags& EngineShowFlags = ViewFamily.EngineShowFlags;
		bool bDrawSimpleCollision = false;
		bool bDrawComplexCollision = false;

		const bool bDrawMesh = true;

		// Draw Mesh
		if (bDrawMesh)
		{
			// Iterate over sections
			for (const FVectorMeshProxySection* Section : Sections)
			{
				if (Section != nullptr)
				{
					FMaterialRenderProxy* MaterialProxy = bWireframe ? WireframeMaterialInstance : Section->Material->GetRenderProxy();

					// For each view..
					for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
					{
						if (VisibilityMap & (1 << ViewIndex))
						{
							const FSceneView* View = Views[ViewIndex];
							// Draw the mesh.
							FMeshBatch& Mesh = Collector.AllocateMesh();
							FMeshBatchElement& BatchElement = Mesh.Elements[0];
							BatchElement.IndexBuffer = &Section->IndexBuffer;
							Mesh.bWireframe = bWireframe;
							Mesh.VertexFactory = &Section->VertexFactory;
							Mesh.MaterialRenderProxy = MaterialProxy;

							bool bHasPrecomputedVolumetricLightmap;
							FMatrix PreviousLocalToWorld;
							int32 SingleCaptureIndex;
							bool bOutputVelocity;
							GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);

							FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
							DynamicPrimitiveUniformBuffer.Set(Collector.GetRHICommandList(), GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, AlwaysHasVelocity());
							BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

							BatchElement.FirstIndex = 0;
							BatchElement.NumPrimitives = Section->IndexBuffer.Indices.Num() / 3;
							BatchElement.MinVertexIndex = 0;
							BatchElement.MaxVertexIndex = Section->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;
							Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
							Mesh.Type = PT_TriangleList;
							Mesh.DepthPriorityGroup = SDPG_World;
							Mesh.bCanApplyViewModeOverrides = false;
							Collector.AddMesh(ViewIndex, Mesh);
						}
					}
				}
			}
		}


#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

		// Draw World Size Rect
		bool bDrawWorldSizeRect = true;
		if (bDrawWorldSizeRect)
		{
			const FVector2D WorldExtent = WorldSize / 2.0f;
			const FVector ActorLocation = GetLocalToWorld().GetOrigin();
			const ESceneDepthPriorityGroup DrawBoundsDPG = EngineShowFlags.Game ? SDPG_World : SDPG_Foreground;
			const float WorldExtentZ = GetBounds().BoxExtent.Z + 1.0f;
			const FVector BoxMax =FVector(WorldExtent.X, WorldExtent.Y, WorldExtentZ);
			const FVector BoxMin = FVector(-WorldExtent.X, -WorldExtent.Y, -WorldExtentZ);
			FBox WorldSizeBox(BoxMin, BoxMax);
			


			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];
					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
					DrawWireBox(PDI, GetLocalToWorld(), WorldSizeBox, FColor::Magenta, DrawBoundsDPG);
				}
			}
		}

		// Draw bounds 
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				// Render bounds
				RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
			}
		}

#endif
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bDynamicRelevance = true;
		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		MaterialRelevance.SetPrimitiveViewRelevance(Result);
		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	virtual uint32 GetMemoryFootprint(void) const
	{
		return(sizeof(*this) + GetAllocatedSize());
	}

	uint32 GetAllocatedSize(void) const
	{
		return(FPrimitiveSceneProxy::GetAllocatedSize());
	}


private:
	/** Array of sections */
	TArray<FVectorMeshProxySection*> Sections;
	FMaterialRelevance MaterialRelevance;
	FVector2D WorldSize;

};

//////////////////////////////////////////////////////////////////////////


UVectorMeshComponent::UVectorMeshComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UVectorMeshComponent::PostLoad()
{
	Super::PostLoad();
}


FColor UVectorMeshComponent::GetWireframeColor() const
{
	if (Mobility == EComponentMobility::Static)
	{
		return FColor(0, 255, 255, 255);
	}
	else if (Mobility == EComponentMobility::Stationary)
	{
		return FColor(128, 128, 255, 255);
	}

	return FColor(255, 0, 255, 255);

}

int32 UVectorMeshComponent::GetNumSections() const
{
	return VectorMeshSections.Num();
}

void UVectorMeshComponent::InitMeshSections(int32 NumSections)
{
	if (VectorMeshSections.Num() != NumSections)
	{
		VectorMeshSections.SetNumZeroed(NumSections);
	}

	for (FVectorShapeMeshSection& Section : VectorMeshSections)
	{
		Section.Reset();
	}
}

void UVectorMeshComponent::ClearMeshSections(bool bMarkDirty)
{
	VectorMeshSections.Empty();
	UpdateLocalBounds();
	MarkRenderStateDirty();
}

void UVectorMeshComponent::MarkMeshDirty()
{
	UpdateLocalBounds();
	MarkRenderStateDirty();
}

void UVectorMeshComponent::UpdateLocalBounds()
{
	FBox LocalBox(ForceInit);

	for (const FVectorShapeMeshSection& Section : VectorMeshSections)
	{
		LocalBox += Section.SectionLocalBox;
	}

	if (AVectorShapeActor* ShapeActor = Cast<AVectorShapeActor>(GetOwner()))
	{
		const FVector2D WorldExtent = ShapeActor->WorldSize / 2.0f;
		const FVector ActorLocation = ShapeActor->GetActorLocation();

		LocalBox += FVector(-WorldExtent.X, WorldExtent.Y, 0);
		LocalBox += FVector(WorldExtent.X, WorldExtent.Y, 0);
		LocalBox += FVector(WorldExtent.X, -WorldExtent.Y, 0);
		LocalBox += FVector(-WorldExtent.X, -WorldExtent.Y, 0);
	}

	LocalBounds = LocalBox.IsValid ? FBoxSphereBounds(LocalBox) : FBoxSphereBounds(FVector(0, 0, 0), FVector(0, 0, 0), 0); // fallback to reset box sphere bounds

	// Update global bounds
	UpdateBounds();
	// Need to send to render thread
	MarkRenderTransformDirty();
}

FPrimitiveSceneProxy* UVectorMeshComponent::CreateSceneProxy()
{
	SCOPE_CYCLE_COUNTER(STAT_VectorMesh_CreateSceneProxy);

	return new FVectorMeshSceneProxy(this);
}

int32 UVectorMeshComponent::GetNumMaterials() const
{
	return VectorMeshSections.Num();
}


FVectorShapeMeshSection* UVectorMeshComponent::GetVectorMeshSection(int32 SectionIndex)
{
	if (SectionIndex < VectorMeshSections.Num())
	{
		return &VectorMeshSections[SectionIndex];
	}
	else
	{
		return nullptr;
	}
}

bool UVectorMeshComponent::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{
	return false;
}

bool UVectorMeshComponent::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
	return false;
}

bool UVectorMeshComponent::WantsNegXTriMesh()
{
	return false;
}

FBoxSphereBounds UVectorMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds Ret(LocalBounds.TransformBy(LocalToWorld));

	Ret.BoxExtent *= BoundsScale;
	Ret.SphereRadius *= BoundsScale;

	return Ret;
}



UBodySetup* UVectorMeshComponent::GetBodySetup()
{
	return nullptr;
}

UMaterialInterface* UVectorMeshComponent::GetMaterialFromCollisionFaceIndex(int32 FaceIndex, int32& SectionIndex) const
{
	UMaterialInterface* Result = nullptr;
	SectionIndex = 0;

	if (FaceIndex >= 0)
	{
		// Look for element that corresponds to the supplied face
		int32 TotalFaceCount = 0;
		for (int32 SectionIdx = 0; SectionIdx < VectorMeshSections.Num(); SectionIdx++)
		{
			const FVectorShapeMeshSection& Section = VectorMeshSections[SectionIdx];
			int32 NumFaces = Section.Indices.Num() / 3;
			TotalFaceCount += NumFaces;

			if (FaceIndex < TotalFaceCount)
			{
				// Grab the material
				Result = GetMaterial(SectionIdx);
				SectionIndex = SectionIdx;
				break;
			}
		}
	}

	return Result;
}
