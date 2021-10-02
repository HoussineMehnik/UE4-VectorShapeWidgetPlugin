//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#include "VectorWidget/SVectorShapeWidget.h"
#include "VectorWidget/SlateVectorShapeData.h"
#include "Rendering/DrawElements.h"
#include "Modules/ModuleManager.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Framework/Application/SlateApplication.h"


#include "SlateMaterialBrush.h"
#include "Math/TransformCalculus2D.h"
#include "VectorShapeTypes.h"




DECLARE_CYCLE_STAT(TEXT("Init Widget Render Data From Asset"), STAT_VectorWidget_InitRenderDataFromVectorAsset, STATGROUP_VectorWidget);
DECLARE_CYCLE_STAT(TEXT("Allocate Widget Vertices"), STAT_VectorWidget_ReallocateVertices, STATGROUP_VectorWidget);


static void VectorMeshToSlateRenderData(const USlateVectorShapeData& DataSource, TArray<FSlateVertex>& OutSlateVerts, TArray<SlateIndex>& OutIndexes)
{
	// Populate Index data
	{
		// Note that we do a slow copy because on some platforms the SlateIndex is
		// a 16-bit value, so we cannot do a memcopy.
		const TArray<uint32>& IndexDataSource = DataSource.GetIndexData();
		const int32 NumIndexes = IndexDataSource.Num();
		OutIndexes.Empty();
		OutIndexes.Reserve(NumIndexes);
		for (int32 i = 0; i < NumIndexes; ++i)
		{
			OutIndexes.Add(IndexDataSource[i]);
		}
	}

	// Populate Vertex Data
	{
		const TArray<FSlateVectorMeshVertex> VertexDataSource = DataSource.GetVertexData();
		const uint32 NumVerts = VertexDataSource.Num();
		OutSlateVerts.Empty();
		OutSlateVerts.Reserve(NumVerts);

		for (uint32 i = 0; i < NumVerts; ++i)
		{
			const FSlateVectorMeshVertex& SourceVertex = VertexDataSource[i];
			FSlateVertex& NewVert = OutSlateVerts[OutSlateVerts.AddUninitialized()];

			// Copy Position
			{
				NewVert.Position[0] = SourceVertex.Position.X;
				NewVert.Position[1] = SourceVertex.Position.Y;
			}

			// Copy Color
			{
				NewVert.Color = FColor(SourceVertex.Color);
			}

			// Copy all the UVs that we have, and as many as we can fit.
			{
				NewVert.TexCoords[0] = 0.5f + NewVert.Position[0] / 2.0f;
				NewVert.TexCoords[1] = 0.5f + NewVert.Position[1] / 2.0f;

				NewVert.TexCoords[2] = NewVert.Position[0] / 2.0f;
				NewVert.TexCoords[3] = NewVert.Position[1] / 2.0f;

				NewVert.MaterialTexCoords[0] = 1.0f;
				NewVert.MaterialTexCoords[1] = 1.0f;
			}
		}
	}
}

void SVectorShapeWidget::Construct(const FArguments& Args)
{
	CachedWorldSize = FVector2D(1, 1);
}

static const FVector2D DummyBrushSize(FVector2D(64, 64));
static const FVector2D DefaultVectorSize(FVector2D(512, 512));

void SVectorShapeWidget::InitRenderData(const USlateVectorShapeData& InVectorDataAsset, UMaterialInterface* InMaterial /*= nullptr*/)
{

	SCOPE_CYCLE_COUNTER(STAT_VectorWidget_InitRenderDataFromVectorAsset);

	{
		SafeDesiredSize = DefaultVectorSize;
		const FVector2D& MeshSize = InVectorDataAsset.GetMeshSize();
		if (!MeshSize.IsZero())
		{
			CachedWorldSize = MeshSize;
			SafeDesiredSize *= (MeshSize / MeshSize.GetMax());
		}
	}

	UMaterialInterface* VectorAssetMaterial = InMaterial;
	if (VectorAssetMaterial == nullptr)
	{
		VectorAssetMaterial = FVectorShapeRuntimeHelpers::GetVectorShapeWidgetDefaultMaterial();
	}

	if (VectorAssetMaterial != nullptr)
	{
		UMaterialInstanceDynamic* NewMID = UMaterialInstanceDynamic::Create(VectorAssetMaterial, nullptr);
		RenderData.Brush = MakeShareable(new FSlateMaterialBrush(*NewMID, DummyBrushSize));
		RenderData.RenderingResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*RenderData.Brush);
	}
	VectorMeshToSlateRenderData(InVectorDataAsset, RenderData.VertexData, RenderData.IndexData);
}


void SVectorShapeWidget::ClearRenderData()
{
	RenderData.VertexData.Empty();
	RenderData.IndexData.Empty();
	RenderData.Brush.Reset();
}

void SVectorShapeWidget::UpdateLinesData(const TArray<FVectorLineData>& InLinesData)
{
	LinesData = InLinesData;
}

void SVectorShapeWidget::ClearLinesData()
{
	LinesData.Empty();
}

void SVectorShapeWidget::SetBrushMaterial(UMaterialInterface* InMaterial)
{
	if (!RenderData.Brush.IsValid())
	{
		RenderData.Brush = MakeShareable(new FSlateMaterialBrush(*FVectorShapeRuntimeHelpers::GetVectorShapeWidgetDefaultMaterial(), DummyBrushSize));
	}

	if (InMaterial != nullptr)
	{
		UMaterialInstanceDynamic* NewMID = UMaterialInstanceDynamic::Create(InMaterial, nullptr);
		RenderData.Brush->SetResourceObject(NewMID);
		RenderData.RenderingResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*RenderData.Brush);
	}
}

UMaterialInstanceDynamic* SVectorShapeWidget::GetMaterialInstanceDynamicSafe()
{

	if (RenderData.IsValid())
	{

		UObject* ResourceObject = RenderData.Brush->GetResourceObject();
		UMaterialInterface* Material = Cast<UMaterialInterface>(ResourceObject);

		if (Material == nullptr)
		{
			Material = FVectorShapeRuntimeHelpers::GetVectorShapeWidgetDefaultMaterial();
		}

		UMaterialInstanceDynamic* ExistingMID = Cast<UMaterialInstanceDynamic>(Material);
		if (ExistingMID == nullptr)
		{
			UMaterialInstanceDynamic* NewMID = UMaterialInstanceDynamic::Create(Material, nullptr);
			RenderData.Brush->SetResourceObject(NewMID);
			RenderData.RenderingResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*RenderData.Brush);
			return NewMID;
		}
		else
		{
			return ExistingMID;
		}
	}
	else
	{
		UMaterialInstanceDynamic* NewMID = UMaterialInstanceDynamic::Create(FVectorShapeRuntimeHelpers::GetVectorShapeWidgetDefaultMaterial(), nullptr);
		RenderData.Brush = MakeShareable(new FSlateMaterialBrush(*NewMID, DummyBrushSize));
		RenderData.RenderingResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*RenderData.Brush);
		return NewMID;
	}

	return nullptr;
}

int32 SVectorShapeWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{

	const FVector2D& GeomLocalExtent = AllottedGeometry.GetLocalSize() * 0.5f;
	const FMatrix2x2& Matrix2D = AllottedGeometry.GetAccumulatedRenderTransform().GetMatrix();
	const FVector2D AbsoluteOffset = AllottedGeometry.LocalToAbsolute(GeomLocalExtent);


	if (RenderData.IsValid())
	{
		{
			TArray<FSlateVertex> TransformedVertexData;
			{
				SCOPE_CYCLE_COUNTER(STAT_VectorWidget_ReallocateVertices);

				TransformedVertexData = RenderData.VertexData;
				for (FSlateVertex& NewSlateVert : TransformedVertexData)
				{
					NewSlateVert.Position = AbsoluteOffset + Matrix2D.TransformPoint(NewSlateVert.Position * GeomLocalExtent);
				}
			}

			FSlateDrawElement::MakeCustomVerts(
				OutDrawElements,
				LayerId,
				RenderData.RenderingResourceHandle,
				TransformedVertexData,
				RenderData.IndexData,
				nullptr, 0, 0,
				ESlateDrawEffect::None);
		}



	}


	if (LinesData.Num() > 0)
	{
		for (const FVectorLineData& LineData : LinesData)
		{
			TArray<FVector2D> TransformedPoints;
			{

				TransformedPoints = LineData.Points;
				for (FVector2D& NewPoint : TransformedPoints)
				{
					NewPoint = AbsoluteOffset + Matrix2D.TransformPoint((2.0f * NewPoint / CachedWorldSize) * GeomLocalExtent);
				}
			}

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				FPaintGeometry(),
				TransformedPoints,
				ESlateDrawEffect::None,
				LineData.Tint,
				LineData.bAntialias,
				LineData.Thickness);
		}
	}


	return LayerId;
}



FVector2D SVectorShapeWidget::ComputeDesiredSize(float) const
{
	return SafeDesiredSize;
}


void SVectorShapeWidget::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (RenderData.Brush.IsValid())
	{
		RenderData.Brush->AddReferencedObjects(Collector);
	}
}
