//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#include "VectorWidget/SlateVectorShapeData.h"
#include "RawIndexBuffer.h"
#include "StaticMeshResources.h"
#include "Engine/StaticMesh.h"
#include "VectorShapeTypes.h"
#include "VectorMeshComponent.h"
#include "UObject/ConstructorHelpers.h"





USlateVectorShapeData::USlateVectorShapeData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MeshSize(ForceInit)
{
	VertexData.Empty();
	IndexData.Empty();
}

const TArray<FSlateVectorMeshVertex>& USlateVectorShapeData::GetVertexData() const
{
	return VertexData;
}

const TArray<uint32>& USlateVectorShapeData::GetIndexData() const
{
	return IndexData;
}

const FVector2D& USlateVectorShapeData::GetMeshSize() const
{
	return MeshSize;
}

void USlateVectorShapeData::InitFromVectorCustomMesh(const FVectorShapeMeshSection* InMeshSection, const FVector2D& InVectorSize)
{
	VertexData.Empty();
	IndexData.Empty();
	MeshSize = InVectorSize;


	if (InVectorSize.X <= 0 || InVectorSize.Y <= 0)
	{
		MeshSize.X = 0.f;
		MeshSize.Y = 0.f;

		UE_LOG(LogVectorShape, Warning, TEXT(" VectorSize is invalid. SVectorShapeWidget expects a valid vector world size."));
		return;
	}

	if (InMeshSection == nullptr)
	{
		UE_LOG(LogVectorShape, Warning, TEXT(" mesh section is null. SVectorShapeWidget expects a valid mesh section."));
	}
	else
	{
		// Populate Vertex Data
		{
			const FVector2D TexCoordOffset(0.5f, 0.5f);
			const FVector2D VectorExtent = MeshSize / 2.0f;
			const uint32 NumVerts = InMeshSection->Vertices.Num();
			VertexData.Empty();
			VertexData.Reserve(NumVerts);

			for (uint32 i = 0; i < NumVerts; ++i)
			{
				// Copy Position
				const FVector& Position = InMeshSection->Vertices[i];

				// Copy Color
				const FColor& Color = InMeshSection->VertexColors[i];

				VertexData.Add(FSlateVectorMeshVertex(
					FVector2D(Position.X, Position.Y) / VectorExtent,
					Color.DWColor()
				));
			}
		}

		// Populate Index data
		{
			const TArray<uint32> SourceIndexes = InMeshSection->Indices;
			const int32 NumIndexes = SourceIndexes.Num();
			IndexData.Empty();
			IndexData.Reserve(NumIndexes);
			for (int32 i = 0; i < NumIndexes; ++i)
			{
				IndexData.Add(SourceIndexes[i]);
			}


			// Sort the index buffer such that verts are drawn in Z-order.
			// Assume that all triangles are coplanar with Z == SomeValue.
			ensure(NumIndexes % 3 == 0);
			for (int32 a = 0; a < NumIndexes; a += 3)
			{
				for (int32 b = 0; b < NumIndexes; b += 3)
				{
					const float VertADepth = InMeshSection->Vertices[IndexData[a]].Z;
					const float VertBDepth = InMeshSection->Vertices[IndexData[b]].Z;
					if (VertADepth < VertBDepth)
					{
						// Swap the order in which triangles will be drawn
						Swap(IndexData[a + 0], IndexData[b + 0]);
						Swap(IndexData[a + 1], IndexData[b + 1]);
						Swap(IndexData[a + 2], IndexData[b + 2]);
					}
				}
			}
		}
	}
}