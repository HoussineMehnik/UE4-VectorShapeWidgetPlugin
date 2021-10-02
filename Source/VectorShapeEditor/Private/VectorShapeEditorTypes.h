//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#pragma once

#include "CoreMinimal.h"
#include "VectorShapeTypes.h"
#include "VectorSplineComponent.h"



struct FVectorShapePolygon
{
	TArray<FVector2D> Vertices2D;
	class UVectorSplineComponent* SplineComponent;

	FVectorShapePolygon(UVectorSplineComponent* InSplineComponent = nullptr)
		: SplineComponent(InSplineComponent)
	{
		Vertices2D.Empty();
	}

	bool IsPolygon() const
	{
		return SplineComponent->SplineType == EVectorSplineType::Polygon;
	}

};
