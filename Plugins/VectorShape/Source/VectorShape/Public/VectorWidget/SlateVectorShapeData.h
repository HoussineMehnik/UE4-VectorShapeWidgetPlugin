// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "SlateVectorShapeData.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMesh;
struct FVectorShapeMeshSection;


USTRUCT()
struct FSlateVectorMeshVertex
{
	GENERATED_USTRUCT_BODY()

		FSlateVectorMeshVertex()
		: Position(ForceInit)
		, Color(0)
	{
	}

	FSlateVectorMeshVertex(
		FVector2D InPos
		, uint32 InColor
	)
		: Position(InPos)
		, Color(InColor)
	{
	}

	UPROPERTY()
		FVector2D Position;

	UPROPERTY()
		uint32 Color;
};

/**
 * Turn static mesh data into Slate's simple vector art format.
 */
UCLASS(BlueprintType)
class VECTORSHAPE_API USlateVectorShapeData : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** Access the slate vertexes. */
	const TArray<FSlateVectorMeshVertex>& GetVertexData() const;
	
	/** Access the indexes for the order in which to draw the vertexes. */
	const TArray<uint32>& GetIndexData() const;

	const FVector2D& GetMeshSize() const;

	void InitFromVectorCustomMesh(const FVectorShapeMeshSection* InMeshSection, const FVector2D& InVectorSize);

private:
	/** @see GetVertexData() */
	UPROPERTY()
	TArray<FSlateVectorMeshVertex> VertexData;

	/** @see GetIndexData() */
	UPROPERTY()
	TArray<uint32> IndexData;

	UPROPERTY()
		FVector2D MeshSize;
};
