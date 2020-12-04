// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/Interface_CollisionDataProvider.h"
#include "Components/MeshComponent.h"
#include "VectorMeshComponent.generated.h"

class FPrimitiveSceneProxy;

/** One section of the VectorMesh. Each material has its own section. */
USTRUCT()
struct FVectorShapeMeshSection
{
	GENERATED_USTRUCT_BODY()

		/** Vertex buffer for this section */
		UPROPERTY()
		TArray<FVector> Vertices;

	UPROPERTY()
		TArray<FVector2D> TextureCoordinates;


	/** Index buffer for this section */
	UPROPERTY()
		TArray<uint32> Indices;

	/** Vertex normal */
	UPROPERTY()
		TArray<FVector> Normals;

	/** Vertex tangent */
	UPROPERTY()
		TArray<FVector> Tangents;

	/** Vertex color */
	UPROPERTY()
		TArray<FColor> VertexColors;

	/** Local bounding box of section */
	UPROPERTY()
		FBox SectionLocalBox;

	FVectorShapeMeshSection()
		: SectionLocalBox(ForceInit)
	{
		Reset();
	}

	/** Reset this section, clear all mesh info. */
	void Reset()
	{
		Vertices.Empty();
		TextureCoordinates.Empty();
		Indices.Empty();
		Tangents.Empty();
		Normals.Empty();
		VertexColors.Empty();
		SectionLocalBox.Init();
	}
	bool IsValid() const
	{
		return
			Vertices.Num() != 0 &&
			TextureCoordinates.Num() != 0 &&
			Indices.Num() != 0 &&
			Tangents.Num() != 0 &&
			Normals.Num() != 0 &&
			VertexColors.Num() != 0;
	}
};

/**
*	Component that allows you to specify custom triangle mesh geometry
*	Beware! This feature is experimental and may be substantially changed in future releases.
*/
UCLASS(hidecategories = (Object, LOD), ShowCategories = (Mobility))
class VECTORSHAPE_API UVectorMeshComponent : public UMeshComponent, public IInterface_CollisionDataProvider
{
	GENERATED_UCLASS_BODY()



public:

	//~ Begin Interface_CollisionDataProvider Interface
	virtual bool GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData) override;
	virtual bool ContainsPhysicsTriMeshData(bool InUseAllTriData) const override;
	virtual bool WantsNegXTriMesh() override;
	//~ End Interface_CollisionDataProvider Interface

//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual class UBodySetup* GetBodySetup() override;
	virtual UMaterialInterface* GetMaterialFromCollisionFaceIndex(int32 FaceIndex, int32& SectionIndex) const override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	virtual int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface.

	//~ Begin UObject Interface
	virtual void PostLoad() override;
	//~ End UObject Interface.

	FColor GetWireframeColor() const;

	/**
	 *	Get pointer to internal data for one section of this VectorMesh component.
	 *	Note that pointer will becomes invalid if sections are added or removed.
	 */
	FVectorShapeMeshSection* GetVectorMeshSection(int32 SectionIndex);

	/** Returns number of sections currently created for this component */
	int32 GetNumSections() const;

	void InitMeshSections(int32 NumSections);

	void ClearMeshSections(bool bMarkDirty);

	void MarkMeshDirty();

private:
	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ Begin USceneComponent Interface.

private:
	/** Update LocalBounds member from the local box of each section */
	void UpdateLocalBounds();

	/** Array of sections of mesh */
	UPROPERTY()
		TArray<FVectorShapeMeshSection> VectorMeshSections;

	/** Local space bounds of mesh */
	UPROPERTY()
		FBoxSphereBounds LocalBounds;

	friend class FVectorMeshSceneProxy;
};


