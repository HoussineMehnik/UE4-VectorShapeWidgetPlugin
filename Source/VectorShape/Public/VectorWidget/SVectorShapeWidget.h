//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "Textures/SlateShaderResource.h"
#include "Rendering/RenderingCommon.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SLeafWidget.h"
#include "VectorShapeWidget.h"
#include "VectorShapeTypes.h"

class FPaintArgs;
class FSlateWindowElementList;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class USlateVectorShapeData;
struct FSlateBrush;



class VECTORSHAPE_API SVectorShapeWidget : public SLeafWidget, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SVectorShapeWidget)
	{}
		SLATE_END_ARGS()

	void Construct(const FArguments& Args);

	void InitRenderData(const USlateVectorShapeData& InVectorDataAsset, UMaterialInterface* InMaterial = nullptr);

	void ClearRenderData();

	void UpdateLinesData(const TArray<FVectorLineData>& InLinesData);

	void ClearLinesData();

	void SetBrushMaterial(UMaterialInterface* InMaterial);

	UMaterialInstanceDynamic* GetMaterialInstanceDynamicSafe();

protected:
	// BEGIN SLeafWidget interface
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	// END SLeafWidget interface

	// ~ FGCObject
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	// ~ FGCObject
	struct FRenderData
	{
		/** Holds a copy of the Static Mesh's data converted to a format that Slate understands. */
		TArray<FSlateVertex> VertexData;
		/** Connectivity data: Order in which the vertexes occur to make up a series of triangles. */
		TArray<SlateIndex> IndexData;
		/** Holds on to the material that is found on the StaticMesh. */
		TSharedPtr<FSlateBrush> Brush;
		/** A rendering handle used to quickly access the rendering data for the slate element*/
		FSlateResourceHandle RenderingResourceHandle;
		
		bool IsValid() const
		{
			return Brush.IsValid() && VertexData.Num() > 0 && IndexData.Num() > 0 ;
		}

	};

	
	FRenderData RenderData;
	FVector2D SafeDesiredSize;
	FVector2D CachedWorldSize;
	TArray<FVectorLineData> LinesData;

};
