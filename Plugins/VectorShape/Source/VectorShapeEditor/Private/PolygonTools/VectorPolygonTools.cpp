

#include "PolygonTools/VectorPolygonTools.h"
#include "VectorShapeActor.h"
#include "VectorSplineComponent.h"
#include "VectorShapeEditorTypes.h"



// Based off "Efficient Polygon Triangulation" algorithm by John W. Ratcliff (http://flipcode.net/archives/Efficient_Polygon_Triangulation.shtml)
bool FPolygonTools::TriangulatePolygon(const TArray<FVector2D>& Polygon, TArray<uint32>& TriangulatedIndices, int32 StartVertex, bool bAllowSimpleTriangulation)
{
	int32 NumVertices = Polygon.Num();
	if (NumVertices < 3)
	{
		return false;
	}

	if (bAllowSimpleTriangulation && IsPolygonConvex(Polygon))
	{
		return TriangulateSimplePolygon(TriangulatedIndices, NumVertices, StartVertex);
	}

	// Allocate and initialize a list of vertex indices for the new polygon
	TArray<int32> TempIndices;
	TempIndices.SetNumUninitialized(NumVertices);
	int32* VertexIndices = TempIndices.GetData();
	for (int32 PointIndex = 0; PointIndex < NumVertices; PointIndex++)
	{
		VertexIndices[PointIndex] = PointIndex;
	}
	

	// Remove NumVertices-2 vertices, creating one triangle every time 
	int32 ErrorDetectionCounter = 2 * NumVertices;
	for (int32 V = NumVertices - 1; NumVertices > 2; )
	{
		// If we loop, it is probably a non-simple polygon
		if (--ErrorDetectionCounter <= 0)
		{
			// Bad polygon!  This can happen if there are adjacent points that are exactly overlapping
			return false;
		}

		// Three consecutive vertices in current polygon, <u,v,w>
		const int32 U = (V < NumVertices) ? V : 0;
		V = ((U + 1) < NumVertices) ? U + 1 : 0;
		const int32 W = ((V + 1) < NumVertices) ? V + 1 : 0;

		if (Snip(Polygon, U, V, W, NumVertices, VertexIndices))
		{
			// Output triangle as indices into the original polygon array
			TriangulatedIndices.Add(StartVertex + VertexIndices[W]);
			TriangulatedIndices.Add(StartVertex + VertexIndices[V]);
			TriangulatedIndices.Add(StartVertex + VertexIndices[U]);
			

			/* Remove V from remaining polygon */
			for (int32 S = V, T = V + 1; T < NumVertices; S++, T++)
			{
				VertexIndices[S] = VertexIndices[T];
			}
			NumVertices--;

			// Reset error detection counter
			ErrorDetectionCounter = 2 * NumVertices;
		}
	}

	return true;
}

bool FPolygonTools::TriangulateSimplePolygon(TArray<uint32>& TriangulatedIndices, int32 NumVertices, int32 StartVertex)
{
	if (NumVertices < 3)
	{
		return false;
	}

	for (int32 Idx = 1; Idx < NumVertices - 1; Idx++)
	{
		new(TriangulatedIndices) int32(StartVertex);
		new(TriangulatedIndices) int32(StartVertex + Idx + 0);
		new(TriangulatedIndices) int32(StartVertex + Idx + 1);
	}
	return  true;
}

void FPolygonTools::RemoveCollinearPoints(TArray<FVector2D>& PointList)
{
	if (PointList.Num() < 3)
	{
		return;
	}

	// Wrap around to get the final pair of vertices (N-1, 0, 1)
	for (int32 VertexIndex = 1; VertexIndex <= PointList.Num() && PointList.Num() >= 3; )
	{
		const FVector2D& A = PointList[VertexIndex - 1];
		const FVector2D& B = PointList[VertexIndex % PointList.Num()];
		const FVector2D& C = PointList[(VertexIndex + 1) % PointList.Num()];

		// Determine if the area of the triangle ABC is zero (if so, they're collinear)
		const float AreaABC = (A.X * (B.Y - C.Y)) + (B.X * (C.Y - A.Y)) + (C.X * (A.Y - B.Y));

		if (FMath::Abs(AreaABC) < KINDA_SMALL_NUMBER)
		{
			// Remove B
			PointList.RemoveAt(VertexIndex % PointList.Num());
		}
		else
		{
			// Continue onwards
			++VertexIndex;
		}
	}
}

void FPolygonTools::RemoveDuplicatedPoints(TArray<FVector2D>& PointList)
{
	if (PointList.Num() < 3)
	{
		return;
	}

	for (int32 VertexIndex = 1; VertexIndex <= PointList.Num() && PointList.Num() > 0; )
	{
		const FVector2D& A = PointList[VertexIndex - 1];
		const FVector2D& B = PointList[VertexIndex % PointList.Num()];

		if ((B - A).SizeSquared() < 1.0f)
		{
			// Remove B
			PointList.RemoveAt(VertexIndex % PointList.Num());
		}
		else
		{
			// Continue onwards
			++VertexIndex;
		}
	}
}

bool FPolygonTools::IsSplineWindingCCW(UVectorSplineComponent* SplineComponent)
{
	float Sum = 0.0f;
	const FInterpCurveVector& SplinePointsPosition = SplineComponent->GetSplinePointsPosition();
	const int PointCount = SplineComponent->GetNumberOfSplinePoints();
	for (int PointIndex = 0; PointIndex < PointCount; ++PointIndex)
	{
		const FVector& A = SplinePointsPosition.Points[PointIndex].OutVal;
		const FVector& B = SplinePointsPosition.Points[(PointIndex + 1) % PointCount].OutVal;
		Sum += (B.X - A.X) * (B.Y + A.Y);
	}
	return (Sum < 0.0f);
}

void FPolygonTools::AddNewPointToPolygon(const FVector& InPoint, FVectorShapePolygon& Polygon, bool bIsCCW)
{
	const int32 NumPoints = Polygon.Vertices2D.Num();
	int32 LastIndex = 0;

	if (bIsCCW)
	{
		LastIndex = Polygon.Vertices2D.Add(FVector2D(InPoint.X, InPoint.Y));
	}
	else
	{
		LastIndex = Polygon.Vertices2D.Insert(FVector2D(InPoint.X, InPoint.Y), 0);
	}
}

void FPolygonTools::SimplifySplinePolygon(FVectorShapePolygon& OutPolygon)
{
	if (UVectorSplineComponent* SplineComponent = OutPolygon.SplineComponent)
	{
		if (AVectorShapeActor* VectorShapeActor = Cast<AVectorShapeActor>(SplineComponent->GetOwner()))
		{
			const float Tolerence = FMath::Lerp<float>(0.25f, 0.9995f, FMath::InterpExpoOut<float>(0, 1, VectorShapeActor->SimplificationThreshold));


			const bool bIsCCW = IsSplineWindingCCW(SplineComponent);
			const int32 NumPoints = SplineComponent->GetNumberOfSplinePoints();
			const FSplineCurves& SplineCurves = SplineComponent->SplineCurves;
			const int32 NumSteps = VectorShapeActor->bUseFixedStepsPerSegment ? VectorShapeActor->StepsPerSegment : SplineCurves.ReparamTable.Points.Num();

			FVector PreviousDirection = FVector::ZeroVector;

			for (int32 PointIdx = 0; PointIdx < NumPoints; PointIdx++)
			{
				const bool bIsLinearPoint = !SplineCurves.Position.Points[PointIdx].IsCurveKey();
				const  FVector LocationAtPoint = SplineCurves.Position.Points[PointIdx].OutVal;
				const FVector DirectionAtPoint = SplineCurves.Position.Points[PointIdx].LeaveTangent.GetSafeNormal();


				AddNewPointToPolygon(LocationAtPoint, OutPolygon, bIsCCW);
				PreviousDirection = DirectionAtPoint;

				if (bIsLinearPoint == true)
				{
					continue;
				}

				for (int32 StepIdx = 1; StepIdx < NumSteps; StepIdx++)
				{
					const float CurrentKey = PointIdx + ((float)StepIdx / float(NumSteps));

					const FVector DirectionAtKey = SplineCurves.Position.EvalDerivative(CurrentKey, FVector::ZeroVector).GetSafeNormal();

					if (FVector::Coincident(PreviousDirection, DirectionAtKey, Tolerence))
					{
						continue;
					}

					const  FVector LocationAtKey = SplineCurves.Position.Eval(CurrentKey, FVector::ZeroVector);
					AddNewPointToPolygon(LocationAtKey, OutPolygon, bIsCCW);
					PreviousDirection = DirectionAtKey;
				}
			}
		}
	}
}

