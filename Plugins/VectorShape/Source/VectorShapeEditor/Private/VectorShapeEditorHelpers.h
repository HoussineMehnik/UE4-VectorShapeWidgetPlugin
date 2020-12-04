#pragma once

#include "CoreMinimal.h"


class UVectorSplineComponent;
class AVectorShapeActor;
struct FVectorShapePolygon;



namespace VectorShapeEditorHelpers
{
	UVectorSplineComponent* AddNewSplineComponent(AVectorShapeActor* VectorShapeActor, bool bIsPolygonShape);
	UVectorSplineComponent* DuplicateSplineComponent(UVectorSplineComponent* SplineComponent);
	void RemoveSplineComponent(UVectorSplineComponent* SplineComponent);
	void ClearAllSplineComponents(AVectorShapeActor* VectorShapeActor);
	void OnCreateVectorMesh(AVectorShapeActor* VectorShapeActor);
}