// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "HitProxies.h"
#include "ComponentVisualizer.h"

class AVectorShapeActor;

/** Base class for clickable spline editing proxies */
/*
struct HNewSplineGizmoVisProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HNewSplineGizmoVisProxy(const UActorComponent* InComponent)
		: HComponentVisProxy(InComponent, HPP_Wireframe)
	{}
};*/



/** SplineComponent visualizer/edit functionality */
class  FVectorMeshComponentVisualizer : public FComponentVisualizer
{
public:
	FVectorMeshComponentVisualizer();
	virtual ~FVectorMeshComponentVisualizer();

	//~ Begin FComponentVisualizer Interface
	virtual void OnRegister() override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual void EndEditing() override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	virtual bool GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual bool HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	//~ End FComponentVisualizer Interface


private:
	/** Actor that owns the currently edited spline */
	TWeakObjectPtr<AVectorShapeActor> VectorShapeActorPtr;

};