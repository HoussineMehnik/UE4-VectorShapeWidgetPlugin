//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
#include "InputCoreTypes.h"
#include "HitProxies.h"
#include "Input/Reply.h"

class AActor;
class FEditorViewportClient;
class FMenuBuilder;
class FPrimitiveDrawInterface;
class FSceneView;
class FUICommandList;
class FViewport;
class SWidget;
class UVectorSplineComponent;
struct FViewportClick;

/** Base class for clickable spline editing proxies */
struct HVectorSplineVisProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HVectorSplineVisProxy(const UActorComponent* InComponent)
	: HComponentVisProxy(InComponent, HPP_Wireframe)
	{}
};

/** Proxy for a spline key */
struct HVectorSplineKeyProxy : public HVectorSplineVisProxy
{
	DECLARE_HIT_PROXY();

	HVectorSplineKeyProxy(const UActorComponent* InComponent, int32 InKeyIndex) 
		: HVectorSplineVisProxy(InComponent)
		, KeyIndex(InKeyIndex)
	{}

	int32 KeyIndex;
};

/** Proxy for a spline segment */
struct HVectorSplineSegmentProxy : public HVectorSplineVisProxy
{
	DECLARE_HIT_PROXY();

	HVectorSplineSegmentProxy(const UActorComponent* InComponent, int32 InSegmentIndex)
		: HVectorSplineVisProxy(InComponent)
		, SegmentIndex(InSegmentIndex)
	{}

	int32 SegmentIndex;
};

/** Proxy for a tangent handle */
struct HVectorSplineTangentHandleProxy : public HVectorSplineVisProxy
{
	DECLARE_HIT_PROXY();

	HVectorSplineTangentHandleProxy(const UActorComponent* InComponent, int32 InKeyIndex, bool bInArriveTangent)
		: HVectorSplineVisProxy(InComponent)
		, KeyIndex(InKeyIndex)
		, bArriveTangent(bInArriveTangent)
	{}

	int32 KeyIndex;
	bool bArriveTangent;
};

/** SplineComponent visualizer/edit functionality */
class  FVectorSplineComponentVisualizer : public FComponentVisualizer
{
public:
	FVectorSplineComponentVisualizer();
	virtual ~FVectorSplineComponentVisualizer();

	//~ Begin FComponentVisualizer Interface
	virtual void OnRegister() override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual void EndEditing() override;
	
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	virtual bool GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual bool HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	virtual TSharedPtr<SWidget> GenerateContextMenu() const override;
	virtual bool IsVisualizingArchetype() const override;
	//~ End FComponentVisualizer Interface

	void ForceRefreshViewports() const;

	/** Get the spline component we are currently editing */
	UVectorSplineComponent* GetEditedSplineComponent() const;

	TSet<int32> GetSelectedKeys() const { return SelectedKeys; }

protected:

	/** Update the key selection state of the visualizer */
	void ChangeSelectionState(int32 Index, bool bIsCtrlHeld);



	void OnSwitchMode();
	bool CanSwitchMode() const;

	void OnSelectAllKeys();
	bool CanSelectAllKeys() const;

	/** Duplicates the selected spline key(s) */
	void DuplicateKey();

	void OnDeleteKey();
	bool CanDeleteKey() const;

	void OnDuplicateKey();
	bool IsKeySelectionValid() const;


	void OnAddKey();
	bool CanAddKey() const;

	bool AreAllKeysSelected() const;

	void OnSetKeyPositionType(EInterpCurveMode Mode);
	bool IsKeyPositionTypeSet(EInterpCurveMode Mode) const;


	void OnResetToDefault();
	bool CanResetToDefault() const;

	void OnSetSplineType(bool bIsAdditive);
	bool IsSplineTypeSet(bool bIsAdditive) const;

	void OnDeleteSplineComponent();

	void OnDuplicateSplineComponent();

	void OnToggleDrawShape();
	bool CanToggleDrawShape() const;
	bool CanDrawShape() const;


	bool HasValidActor() const;

	void OnApplyDefaultColor();
	

	/** Generate the submenu containing the available point types */
	void GenerateSplinePointTypeSubMenu(FMenuBuilder& MenuBuilder) const;

	void GenerateSplineTypeSubMenu(FMenuBuilder& MenuBuilder) const;


	TSharedPtr<SWidget> CreateColorSelectionWidegt() const;

	/**
	* @return The color that should be displayed in the color block
	*/
	FLinearColor OnGetColorForColorBlock() const;

	/**
	* Called when the user clicks in the color block (opens inline color picker)
	*/
	FReply OnMouseButtonDownColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;

	/**
	* Creates a new color picker for interactively selecting the color
	*
	* @param bUseAlpha If true alpha will be displayed, otherwise it is ignored
	* @param bOnlyRefreshOnOk If true the value of the property will only be refreshed when the user clicks OK in the color picker
	*/
	void CreateColorPicker(bool bUseAlpha) const;

	/**
	* Called when the property is set from the color picker
	*
	* @param NewColor The new color value
	*/
	void OnSetColorFromColorPicker(FLinearColor NewColor) const;

	/**
	* Called when the user clicks cancel in the color picker
	* The values are reset to their original state when this happens
	*
	* @param OriginalColor Original color of the property
	*/
	void OnColorPickerCancelled(FLinearColor OriginalColor) const;

	/**
	* Called when the user enters an interactive color change (dragging something in the picker)
	*/
	void OnColorPickerInteractiveBegin() const;

	/**
	* Called when the user completes an interactive color change (dragging something in the picker)
	*/
	void OnColorPickerInteractiveEnd() const;




	void CloseMenu() const;


	struct ESelectedTangentHandle
	{
		enum Type
		{
			None,
			Leave,
			Arrive
		};
	};

	/** Index of the last key we selected */
	int32 LastKeyIndexSelected;

	/** Index of segment we have selected */
	int32 SelectedSegmentIndex;

	/** Index of tangent handle we have selected */
	int32 SelectedTangentHandle;


	/** The type of the selected tangent handle */
	ESelectedTangentHandle::Type SelectedTangentHandleType;

	/** Whether we currently allow duplication when dragging */
	bool bAllowDuplication;

	private:

		bool bAltPressed;
		bool bCtrlPressed;
		bool bShiftPressed;

protected:

	FIntPoint SelectedSegment;

	/** Cached rotation for this point */
	FQuat CachedRotation;

	struct ESplineEditMode
	{
		enum Type
		{
			EditMode,
			TransformMode,
		};
	};

	ESplineEditMode::Type CurrentEditMode;

	/** Output log commands */
	TSharedPtr<FUICommandList> SplineComponentVisualizerActions;

	/** Actor that owns the currently edited spline */
	TWeakObjectPtr<AActor> SplineOwningActor;

	/** Name of property on the actor that references the spline we are editing */
	FComponentPropertyPath SplinePropertyPath;

	/** Index of keys we have selected */
	TSet<int32> SelectedKeys;

	/** Position on spline we have selected */
	FVector SelectedSplinePosition;



private:
	FProperty* SplineCurvesProperty;


};
