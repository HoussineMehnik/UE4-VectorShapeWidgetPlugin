//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#include "VectorSplineComponentVisualizer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/InputChord.h"
#include "Framework/Commands/Commands.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EditorStyleSet.h"
#include "UnrealWidget.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "ScopedTransaction.h"
#include "ActorEditorUtils.h"
#include "VectorSplineComponent.h"
#include "VectorShapeTypes.h"
#include "VectorShapeEditorHelpers.h"
#include "VectorShapeActor.h"
#include "VectorShapeEditorStyle.h"
#include "ContentBrowserDelegates.h"
#include "CanvasItem.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Colors/SColorPicker.h"
#include "Engine/Engine.h"
#include "PolygonTools/VectorPolygonTools.h"
#include "SceneView.h"
#include "Settings/LevelEditorViewportSettings.h"

IMPLEMENT_HIT_PROXY(HVectorSplineVisProxy, HComponentVisProxy);
IMPLEMENT_HIT_PROXY(HVectorSplineKeyProxy, HVectorSplineVisProxy);
IMPLEMENT_HIT_PROXY(HVectorSplineSegmentProxy, HVectorSplineVisProxy);
IMPLEMENT_HIT_PROXY(HVectorSplineTangentHandleProxy, HVectorSplineVisProxy);

#define LOCTEXT_NAMESPACE "VectorSplineComponentVisualizer"

/** Define commands for the spline component visualizer */
class FVectorSplineComponentVisualizerCommands : public TCommands<FVectorSplineComponentVisualizerCommands>
{
public:
	FVectorSplineComponentVisualizerCommands() : TCommands <FVectorSplineComponentVisualizerCommands>
	(
		"VectorShapeEditor",	// Context name for fast lookup
		LOCTEXT( "VectorShapeEditor", "Vector Shape Editor"),	// Localized context name for displaying
		NAME_None,	// Parent
		FVectorShapeEditorStyle::Get()->GetStyleSetName()
		)
	{
	}

	virtual void RegisterCommands() override
	{
		UI_COMMAND(SwitchMode, "Switch Mode", "Switch Edit Mode to Edit or Transform Mode .", EUserInterfaceActionType::Button, FInputChord(EKeys::Tab));

		UI_COMMAND(SelectAllKeys, "Select All Spline Points", "Set spline points selected.", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::A));
		UI_COMMAND(DeleteKey, "Delete Spline Point", "Delete the currently selected spline point.", EUserInterfaceActionType::Button, FInputChord(EKeys::Delete));
		UI_COMMAND(DuplicateKey, "Duplicate Spline Point", "Duplicate the currently selected spline point.", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(AddKey, "Add Spline Point Here", "Add a new spline point at the cursor location.", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::D));

		UI_COMMAND(SetKeyPositionToLinear, "Linear", "Set spline point position to Linear type", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::NumPadOne));
		UI_COMMAND(SetKeyPositionToCurve, "Curve", "Set spline point position to Curve type", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::NumPadTwo));
		UI_COMMAND(SetKeyPositionToCurveAuto, "Curve", "Set spline point position to CurveAuto type", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::NumPadThree));
		UI_COMMAND(SetKeyPositionToCurveClamped, "Clamped Tangent", "Reset the tangent for this spline point to its default clamped value.", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::NumPadFour));
		UI_COMMAND(SetKeyPositionToCurveBreak, "Curve Break", "Set spline point position to break type", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::NumPadFive));


		UI_COMMAND(SetSplineToPolygon, "Polygon", "Set spline shape type to Polygon.", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::Add));
		UI_COMMAND(SetSplineToLine, "Line", "Set spline shape type to Line.", EUserInterfaceActionType::RadioButton, FInputChord(EKeys::Subtract));
		UI_COMMAND(DuplicateSpline, "Duplicate Spline", "Duplicate the currently selected spline component.", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::W));
		UI_COMMAND(DeleteSpline, "Delete Spline", "Delete the selected spline component.", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::Delete));
		UI_COMMAND(ResetToDefault, "Reset to Default", "Reset this spline to its archetype default.", EUserInterfaceActionType::Button, FInputChord());

		UI_COMMAND(DrawShape, "Draw Shape", "Should render or not this spline shape.", EUserInterfaceActionType::ToggleButton, FInputChord());
		UI_COMMAND(ApplyDefaultColor, "Apply Default Color", "Apply shape actor default color to this spline.", EUserInterfaceActionType::Button, FInputChord());

	}

public:


	TSharedPtr<FUICommandInfo> SwitchMode;

	/** Select all keys */
		TSharedPtr<FUICommandInfo>  SelectAllKeys;

	/** Delete key */
	TSharedPtr<FUICommandInfo> DeleteKey;

	/** Duplicate key */
	TSharedPtr<FUICommandInfo> DuplicateKey;

	/** Add key */
	TSharedPtr<FUICommandInfo> AddKey;

	/** Reset to clamped tangent **/
	TSharedPtr<FUICommandInfo> SetKeyPositionToCurveClamped;

	/** Set spline key to Curve type */
	TSharedPtr<FUICommandInfo> SetKeyPositionToCurveAuto;

	/** Set spline key to Curve type */
	TSharedPtr<FUICommandInfo> SetKeyPositionToCurve;

	/** Set spline key to Linear type */
	TSharedPtr<FUICommandInfo> SetKeyPositionToLinear;

	TSharedPtr<FUICommandInfo> SetKeyPositionToCurveBreak;

	/** Reset this spline to its default */
	TSharedPtr<FUICommandInfo> ResetToDefault;

	TSharedPtr<FUICommandInfo> SetSplineToPolygon;

	TSharedPtr<FUICommandInfo> SetSplineToLine;

	TSharedPtr<FUICommandInfo> DrawShape;

	TSharedPtr<FUICommandInfo> ApplyDefaultColor;

	TSharedPtr<FUICommandInfo> DuplicateSpline;

	TSharedPtr<FUICommandInfo> DeleteSpline;


};



FVectorSplineComponentVisualizer::FVectorSplineComponentVisualizer()
	: FComponentVisualizer()
	, LastKeyIndexSelected(INDEX_NONE)
	, SelectedSegmentIndex(INDEX_NONE)
	, SelectedTangentHandle(INDEX_NONE)
	, SelectedTangentHandleType(ESelectedTangentHandle::None)
	, bAllowDuplication(true)
	, bAltPressed(false)
	, bCtrlPressed(false)
	, bShiftPressed(false)
	, SelectedSegment(INDEX_NONE, INDEX_NONE)
	, CachedRotation(FQuat::Identity)
	, CurrentEditMode(ESplineEditMode::EditMode)
{
	FVectorSplineComponentVisualizerCommands::Register();

	SplineComponentVisualizerActions = MakeShareable(new FUICommandList);

	SplineCurvesProperty = FindFProperty<FProperty>(UVectorSplineComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UVectorSplineComponent, SplineCurves));
}

void FVectorSplineComponentVisualizer::OnRegister()
{
	const auto& Commands = FVectorSplineComponentVisualizerCommands::Get();


	SplineComponentVisualizerActions->MapAction(
		Commands.SwitchMode,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnSwitchMode),
		FCanExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::CanSwitchMode));

	SplineComponentVisualizerActions->MapAction(
		Commands.SelectAllKeys,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnSelectAllKeys),
		FCanExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::CanSelectAllKeys));

	SplineComponentVisualizerActions->MapAction(
		Commands.DeleteKey,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnDeleteKey),
		FCanExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::CanDeleteKey));

	SplineComponentVisualizerActions->MapAction(
		Commands.DuplicateKey,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnDuplicateKey),
		FCanExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::IsKeySelectionValid));

	SplineComponentVisualizerActions->MapAction(
		Commands.AddKey,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnAddKey),
		FCanExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::CanAddKey));

	SplineComponentVisualizerActions->MapAction(
		Commands.SetSplineToPolygon,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnSetSplineType, true),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FVectorSplineComponentVisualizer::IsSplineTypeSet, true));

	SplineComponentVisualizerActions->MapAction(
		Commands.SetSplineToLine,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnSetSplineType, false),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FVectorSplineComponentVisualizer::IsSplineTypeSet, false));


	SplineComponentVisualizerActions->MapAction(
		Commands.SetKeyPositionToLinear,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnSetKeyPositionType, CIM_Linear),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FVectorSplineComponentVisualizer::IsKeyPositionTypeSet, CIM_Linear));

	SplineComponentVisualizerActions->MapAction(
		Commands.SetKeyPositionToCurve,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnSetKeyPositionType, CIM_CurveUser),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FVectorSplineComponentVisualizer::IsKeyPositionTypeSet, CIM_CurveUser));

	SplineComponentVisualizerActions->MapAction(
		Commands.SetKeyPositionToCurveAuto,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnSetKeyPositionType, CIM_CurveAuto),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FVectorSplineComponentVisualizer::IsKeyPositionTypeSet, CIM_CurveAuto));

	SplineComponentVisualizerActions->MapAction(
		Commands.SetKeyPositionToCurveClamped,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnSetKeyPositionType, CIM_CurveAutoClamped),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FVectorSplineComponentVisualizer::IsKeyPositionTypeSet, CIM_CurveAutoClamped));

	SplineComponentVisualizerActions->MapAction(
		Commands.SetKeyPositionToCurveBreak,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnSetKeyPositionType, CIM_CurveBreak),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FVectorSplineComponentVisualizer::IsKeyPositionTypeSet, CIM_CurveBreak));

	SplineComponentVisualizerActions->MapAction(
		Commands.ResetToDefault,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnResetToDefault),
		FCanExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::CanResetToDefault));



	SplineComponentVisualizerActions->MapAction(
		Commands.DeleteSpline,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnDeleteSplineComponent));

	SplineComponentVisualizerActions->MapAction(
		Commands.DuplicateSpline,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnDuplicateSplineComponent));


	SplineComponentVisualizerActions->MapAction(
		Commands.DrawShape,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnToggleDrawShape),
		FCanExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::CanToggleDrawShape),
		FIsActionChecked::CreateSP(this, &FVectorSplineComponentVisualizer::CanDrawShape));


	SplineComponentVisualizerActions->MapAction(
		Commands.ApplyDefaultColor,
		FExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::OnApplyDefaultColor),
		FCanExecuteAction::CreateSP(this, &FVectorSplineComponentVisualizer::HasValidActor));


}

FVectorSplineComponentVisualizer::~FVectorSplineComponentVisualizer()
{
	FVectorSplineComponentVisualizerCommands::Unregister();
}

static FIntPoint FindSegmentClosestToWorldLocation(const UVectorSplineComponent* SplineComp, const FVector& WorldLocation)
{
	const int32 NumPoints = SplineComp->GetNumberOfSplinePoints();

	if (NumPoints == 1)
	{
		return FIntPoint(NumPoints, NumPoints);
	}

	float NearestDistance = FLT_MAX;
	int32 PointA = INDEX_NONE;

	for (int32 PointIdx = 0; PointIdx < NumPoints; PointIdx++)
	{
		const FVector PointLocation = SplineComp->GetWorldLocationAtSplinePoint(PointIdx);
		const float TempDistance = FVector::Distance(WorldLocation, PointLocation);

		if (TempDistance < NearestDistance)
		{
			NearestDistance = TempDistance;
			PointA = PointIdx;
		}
	}

	if (PointA == INDEX_NONE)
	{
		return FIntPoint(INDEX_NONE, INDEX_NONE);
	}

	//const bool bIsClosedLoop = SplinesCurves[CurvesIndex].IsClosedLoop();

	const FVector NearestPointLocation = SplineComp->GetWorldLocationAtSplinePoint(PointA);
	const FVector PreviousPointLocation = SplineComp->GetWorldLocationAtSplinePoint(PointA - 1);
	const FVector NextPointLocation = SplineComp->GetWorldLocationAtSplinePoint(PointA + 1);

	const FVector ToHit = (WorldLocation - NearestPointLocation);
	const FVector ToPrev = (PreviousPointLocation - NearestPointLocation);
	const FVector ToNext = (NextPointLocation - NearestPointLocation);

	if ((ToHit | ToPrev) > (ToHit | ToNext))
	{
		if (PointA == 0)
		{
			return FIntPoint(NumPoints - 1, PointA);
		}
		else
		{
			return FIntPoint(PointA - 1, PointA);
		}
	}
	else
	{
		if (PointA == NumPoints - 1)
		{
			return FIntPoint(PointA, 0);
		}
		else
		{
			return FIntPoint(PointA, PointA + 1);
		}
	}

	return FIntPoint(INDEX_NONE, INDEX_NONE);
}

static void CalculateCursorTraceStartEnd(FEditorViewportClient* InViewportClient, FVector& Start, FVector& End)
{
	Start = FVector(0.f, 0.f, 0.f);
	End = FVector(0.f, 0.f, 0.f);

	// Cache a copy of the world pointer	
	UWorld* World = InViewportClient->GetWorld();

	if (World == nullptr)
	{
		return;
	}

	// Compute a world space ray from the screen space mouse coordinates
	FSceneViewFamilyContext ViewFamily(FSceneViewFamilyContext::ConstructionValues(InViewportClient->Viewport, InViewportClient->GetScene(), InViewportClient->EngineShowFlags)
		.SetRealtimeUpdate(InViewportClient->IsRealtime()));

	FSceneView* View = InViewportClient->CalcSceneView(&ViewFamily);
	int32 MouseX = InViewportClient->Viewport->GetMouseX();
	int32 MouseY = InViewportClient->Viewport->GetMouseY();
	FViewportCursorLocation MouseViewportRay(View, InViewportClient, MouseX, MouseY);
	FVector MouseViewportRayDirection = MouseViewportRay.GetDirection();

	Start = MouseViewportRay.GetOrigin();
	End = Start + WORLD_MAX * MouseViewportRayDirection;
	if (InViewportClient->IsOrtho())
	{
		Start -= WORLD_MAX * MouseViewportRayDirection;
	}
}

void FVectorSplineComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if (FViewport* CurrentViewport = GEditor->GetActiveViewport())
	{
		if (FEditorViewportClient* EditorViewportClient = (FEditorViewportClient*)CurrentViewport->GetClient())
		{
			bCtrlPressed = EditorViewportClient->IsCtrlPressed();
			bAltPressed = EditorViewportClient->IsAltPressed();
			bShiftPressed = EditorViewportClient->IsShiftPressed();
		}
		else /*if (bCtrlPressed || bAltPressed || bShiftPressed)*/
		{
			bCtrlPressed = false;
			bAltPressed = false;
			bShiftPressed = false;
		}
	}
	else /*if (bCtrlPressed || bAltPressed || bShiftPressed)*/
	{
		bCtrlPressed = false;
		bAltPressed = false;
		bShiftPressed = false;
	}

	if (const UVectorSplineComponent* SplineComp = Cast<const UVectorSplineComponent>(Component))
	{
		const FInterpCurveVector& SplineInfo = SplineComp->GetSplinePointsPosition();
		const UVectorSplineComponent* EditedSplineComp = GetEditedSplineComponent();

		const UVectorSplineComponent* Archetype = CastChecked<UVectorSplineComponent>(SplineComp->GetArchetype());
		const bool bIsSplineEditable = !SplineComp->bModifiedByConstructionScript; // bSplineHasBeenEdited || SplineInfo == Archetype->SplineCurves.Position || SplineComp->bInputSplinePointsToConstructionScript;
		const bool bIsPolygon = (SplineComp->SplineType == EVectorSplineType::Polygon);

		const FColor ReadOnlyColor = FColor(255, 0, 255, 255);
		const FColor NormalColor = bIsSplineEditable ? FColor(SplineComp->SplineColor) : ReadOnlyColor;
		const FColor SelectedColor = bIsSplineEditable ? FColor(SplineComp->SelectionColor.ToFColor(true)) : ReadOnlyColor;
		const float GrabHandleSize = 12.0f;
		const float TangentHandleSize = 10.0f;

		
		const float& VectorShapeOffsetZ = (SplineComp->OffsetZ + 0.01f);
		FVector PositionOffset = VectorShapeOffsetZ * SplineComp->GetUpVector();


		// Draw the tangent handles before anything else so they will not overdraw the rest of the spline
		if (SplineComp == EditedSplineComp && (CurrentEditMode == ESplineEditMode::EditMode))
		{
			for (int32 SelectedKey : SelectedKeys)
			{
				if (SplineInfo.Points[SelectedKey].InterpMode == CIM_CurveBreak || SplineInfo.Points[SelectedKey].InterpMode == CIM_CurveUser)
				{
					const bool bIsCurveBreak = (SplineInfo.Points[SelectedKey].InterpMode == CIM_CurveBreak);
					const FVector Location = SplineComp->GetLocationAtSplinePoint(SelectedKey, ESplineCoordinateSpace::World) + PositionOffset;
					const FVector LeaveTangent = SplineComp->GetLeaveTangentAtSplinePoint(SelectedKey, ESplineCoordinateSpace::World);
					const FVector ArriveTangent = bIsCurveBreak ? SplineComp->GetArriveTangentAtSplinePoint(SelectedKey, ESplineCoordinateSpace::World) : LeaveTangent;

					PDI->SetHitProxy(NULL);


					DrawDashedLine(PDI, Location, Location + LeaveTangent, SelectedColor, bIsCurveBreak ? 25.0f : 10.0f, SDPG_Foreground);


					DrawDashedLine(PDI, Location, Location - ArriveTangent, SelectedColor, bIsCurveBreak ? 12.5f : 10.0f, SDPG_Foreground);


					if (bIsSplineEditable)
					{
						PDI->SetHitProxy(new HVectorSplineTangentHandleProxy(Component, SelectedKey, false));
					}
					PDI->DrawPoint(Location + LeaveTangent, SelectedColor, bIsCurveBreak ? TangentHandleSize * 1.25 : TangentHandleSize, SDPG_Foreground);

					if (bIsSplineEditable)
					{
						PDI->SetHitProxy(new HVectorSplineTangentHandleProxy(Component, SelectedKey, true));
					}
					PDI->DrawPoint(Location - ArriveTangent, SelectedColor, bIsCurveBreak ? TangentHandleSize * 1.1f : TangentHandleSize, SDPG_Foreground);

					PDI->SetHitProxy(NULL);
				}
			}
		}
		const float DefaultScale = SplineComp->ScaleVisualizationWidth;

		FVector OldKeyPos(0);
		FVector OldKeyScale(0);

		const int32 NumPoints = SplineInfo.Points.Num();
		const int32 NumSegments = SplineInfo.bIsLooped ? NumPoints : NumPoints - 1;
		for (int32 KeyIdx = 0; KeyIdx < NumSegments + 1; KeyIdx++)
		{
			const FVector NewKeyPos = SplineComp->GetLocationAtSplinePoint(KeyIdx, ESplineCoordinateSpace::World) + PositionOffset;
			const FVector NewKeyUpVector = SplineComp->GetUpVectorAtSplinePoint(KeyIdx, ESplineCoordinateSpace::World);
			const FVector NewKeyScale = SplineComp->GetScaleAtSplinePoint(KeyIdx) * DefaultScale;

			const FColor KeyColor = (SplineComp == EditedSplineComp && ((CurrentEditMode == ESplineEditMode::TransformMode) || SelectedKeys.Contains(KeyIdx))) ? SelectedColor : NormalColor;

			// Draw the keypoint and up/right vectors
			if (KeyIdx < NumPoints)
			{

				if (bIsSplineEditable)
				{
					PDI->SetHitProxy(new HVectorSplineKeyProxy(Component, KeyIdx));
				}
				PDI->DrawPoint(NewKeyPos, KeyColor, GrabHandleSize, SDPG_Foreground);
				PDI->SetHitProxy(NULL);
			}

			// If not the first keypoint, draw a line to the previous keypoint.
			if (KeyIdx > 0)
			{
				const FColor LineColor = (SplineComp == EditedSplineComp && ((CurrentEditMode == ESplineEditMode::TransformMode) || SelectedKeys.Contains(KeyIdx - 1))) ? SelectedColor : NormalColor;
				if (bIsSplineEditable)
				{
					PDI->SetHitProxy(new HVectorSplineSegmentProxy(Component, KeyIdx - 1));
				}

				{
					// Find position on first keyframe.
					FVector OldPos = OldKeyPos;
					FVector OldScale = OldKeyScale;

					// Then draw a line for each substep.
					const int32 NumSteps = 20;

					for (int32 StepIdx = 1; StepIdx <= NumSteps; StepIdx++)
					{
						const float Key = (KeyIdx - 1) + (StepIdx / static_cast<float>(NumSteps));
						const FVector NewPos = SplineComp->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World) + PositionOffset;
						const FVector NewScale = SplineComp->GetScaleAtSplineInputKey(Key) * DefaultScale;

						PDI->DrawLine(OldPos, NewPos, LineColor, SDPG_Foreground);

						OldPos = NewPos;
						OldScale = NewScale;
					}
				}

				PDI->SetHitProxy(NULL);
			}

			OldKeyPos = NewKeyPos;
			OldKeyScale = NewKeyScale;
		}

		if (SplineComp == EditedSplineComp)
		{
			const FVector VectorShapeLocation = SplineComp->GetComponentLocation();
			const FVector VectorShapeUpVector = SplineComp->GetUpVector();
			AActor* VectorShapeActor = SplineComp->GetOwner();

			FVector LineStart(ForceInitToZero);
			FVector LineEnd(ForceInitToZero);
			FVector Instersection(ForceInitToZero);
			if (bCtrlPressed && SplineComp == EditedSplineComp)
			{
				if (FViewport* CurrentViewport = GEditor->GetActiveViewport())
				{
					if (FEditorViewportClient* EditorViewportClient = (FEditorViewportClient*)CurrentViewport->GetClient())
					{
						CalculateCursorTraceStartEnd(EditorViewportClient, LineStart, LineEnd);

						const FVector LineStartLocal(VectorShapeActor->GetActorTransform().InverseTransformPosition(LineStart));
						const FVector LineEndLocal(VectorShapeActor->GetActorTransform().InverseTransformPosition(LineEnd));

						if (LineStartLocal.Z * LineEndLocal.Z < 0)
						{
							
							Instersection = FMath::LinePlaneIntersection(LineStart, LineEnd, VectorShapeLocation , VectorShapeUpVector);
							
						}
					}
				}
				if (bAltPressed)
				{
					SelectedSegment = FindSegmentClosestToWorldLocation(SplineComp, Instersection);
				}
			}
			else if (!bCtrlPressed && SelectedSegment != FIntPoint::NoneValue)
			{
				SelectedSegment = FIntPoint(INDEX_NONE, INDEX_NONE);
			}

			if (SelectedSegment.X != INDEX_NONE && SelectedSegment.X != SelectedSegment.Y)
			{
				if (GetDefault<ULevelEditorViewportSettings>()->GridEnabled)
				{
					Instersection = Instersection.GridSnap(GEditor->GetGridSize());
				}

				const FVector PosA = SplineComp->GetWorldLocationAtSplinePoint(SelectedSegment.X);
				const FVector PosB = SplineComp->GetWorldLocationAtSplinePoint(SelectedSegment.Y);


				if (FVector::Distance(PosA, Instersection) > DefaultScale / 2.0f && FVector::Distance(PosB, Instersection) > DefaultScale / 2.0f)
				{

					DrawDashedLine(PDI, PosA + VectorShapeUpVector * VectorShapeOffsetZ, Instersection + VectorShapeUpVector * VectorShapeOffsetZ, NormalColor, 10, SDPG_Foreground);
					DrawDashedLine(PDI, PosB + VectorShapeUpVector * VectorShapeOffsetZ, Instersection + VectorShapeUpVector * VectorShapeOffsetZ, NormalColor, 10, SDPG_Foreground);

					PDI->DrawPoint(Instersection + VectorShapeUpVector * VectorShapeOffsetZ, NormalColor, GrabHandleSize, SDPG_Foreground);

					SelectedSplinePosition = Instersection;
				}
				else if (SelectedSegment != FIntPoint::NoneValue)
				{
					SelectedSegment = FIntPoint(INDEX_NONE, INDEX_NONE);
				}
			}

		}

	}
	else if (SelectedSegment != FIntPoint::NoneValue)
	{
		SelectedSegment = FIntPoint(INDEX_NONE, INDEX_NONE);
	}
}

void FVectorSplineComponentVisualizer::DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	if (FEditorViewportClient* EditorViewport = (FEditorViewportClient*)Viewport->GetClient())
	{
		if (UVectorSplineComponent* SplineComponent = GetEditedSplineComponent())
		{

			if (SplineComponent != Component)
			{
				return;
			}

			int32 YPos = 42;

			{
				const FText TitleTxt = (CurrentEditMode == ESplineEditMode::EditMode) ?
					LOCTEXT("SplineEditMode", "Spline Visualizer : Edit Mode") : LOCTEXT("SplineEditMode", "Spline Visualizer : Transform Mode");

				FCanvasTextItem TextItem(FVector2D(6, YPos), TitleTxt, GEngine->GetSmallFont(), FLinearColor::White);
				TextItem.EnableShadow(FLinearColor::Black);
				TextItem.Draw(Canvas);
				YPos += 18;
			}
		}

	}
}

void FVectorSplineComponentVisualizer::ChangeSelectionState(int32 Index, bool bIsCtrlHeld)
{
	if (Index == INDEX_NONE)
	{
		SelectedKeys.Empty();
		LastKeyIndexSelected = INDEX_NONE;
	}
	else if (!bIsCtrlHeld)
	{
		SelectedKeys.Empty();
		SelectedKeys.Add(Index);
		LastKeyIndexSelected = Index;
	}
	else
	{
		// Add or remove from selection if Ctrl is held
		if (SelectedKeys.Contains(Index))
		{
			// If already in selection, toggle it off
			SelectedKeys.Remove(Index);

			if (LastKeyIndexSelected == Index)
			{
				if (SelectedKeys.Num() == 0)
				{
					// Last key selected: clear last key index selected
					LastKeyIndexSelected = INDEX_NONE;
				}
				else
				{
					// Arbitarily set last key index selected to first member of the set (so that it is valid)
					LastKeyIndexSelected = *SelectedKeys.CreateConstIterator();
				}
			}
		}
		else
		{
			// Add to selection
			SelectedKeys.Add(Index);
			LastKeyIndexSelected = Index;
		}
	}
}



bool FVectorSplineComponentVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	if(VisProxy && VisProxy->Component.IsValid())
	{
		const UVectorSplineComponent* SplineComp = CastChecked<const UVectorSplineComponent>(VisProxy->Component.Get());

		SplinePropertyPath = FComponentPropertyPath(SplineComp);
		if(SplinePropertyPath.IsValid())
		{
			AActor* OldSplineOwningActor = SplineOwningActor.Get();
			SplineOwningActor = SplineComp->GetOwner();

			if (OldSplineOwningActor != SplineOwningActor)
			{
				// Reset selection state if we are selecting a different actor to the one previously selected
				ChangeSelectionState(INDEX_NONE, false);
				SelectedSegmentIndex = INDEX_NONE;
				SelectedTangentHandle = INDEX_NONE;
				SelectedTangentHandleType = ESelectedTangentHandle::None;
			}

			if (VisProxy->IsA(HVectorSplineKeyProxy::StaticGetType()))
			{
				// Control point clicked

				HVectorSplineKeyProxy* KeyProxy = (HVectorSplineKeyProxy*)VisProxy;

				// Modify the selection state, unless right-clicking on an already selected key
				if (Click.GetKey() != EKeys::RightMouseButton || !SelectedKeys.Contains(KeyProxy->KeyIndex))
				{
					if (!InViewportClient->IsAltPressed() &&
						InViewportClient->IsShiftPressed())
					{
						if (!InViewportClient->IsCtrlPressed() && SplineComp->GetNumberOfSplinePoints() > 3)
						{
							ChangeSelectionState(KeyProxy->KeyIndex, false);
							OnDeleteKey();
						}
						else if (InViewportClient->IsCtrlPressed())
						{
							OnDeleteSplineComponent();
						}

					}
					else
					{
						ChangeSelectionState(KeyProxy->KeyIndex, InViewportClient->IsCtrlPressed());
					}
				}
				SelectedSegmentIndex = INDEX_NONE;
				SelectedTangentHandle = INDEX_NONE;
				SelectedTangentHandleType = ESelectedTangentHandle::None;

				if (LastKeyIndexSelected == INDEX_NONE)
				{
					SplineOwningActor = nullptr;
					return false;
				}

				if (LastKeyIndexSelected != INDEX_NONE)
				{
					CachedRotation = SplineComp->GetQuaternionAtSplinePoint(LastKeyIndexSelected, ESplineCoordinateSpace::World);
				}

				return true;
			}
			else if (VisProxy->IsA(HVectorSplineSegmentProxy::StaticGetType()))
			{
				// Spline segment clicked

				// Divide segment into subsegments and test each subsegment against ray representing click position and camera direction.
				// Closest encounter with the spline determines the spline position.
				const int32 NumSubdivisions = 16;

				HVectorSplineSegmentProxy* SegmentProxy = (HVectorSplineSegmentProxy*)VisProxy;
				ChangeSelectionState(SegmentProxy->SegmentIndex, InViewportClient->IsCtrlPressed());
				SelectedSegmentIndex = SegmentProxy->SegmentIndex;
				SelectedTangentHandle = INDEX_NONE;
				SelectedTangentHandleType = ESelectedTangentHandle::None;

				if (LastKeyIndexSelected == INDEX_NONE)
				{
					SplineOwningActor = nullptr;
					return false;
				}

				CachedRotation = SplineComp->GetQuaternionAtSplinePoint(LastKeyIndexSelected, ESplineCoordinateSpace::World);

				float SubsegmentStartKey = static_cast<float>(SelectedSegmentIndex);
				FVector SubsegmentStart = SplineComp->GetLocationAtSplineInputKey(SubsegmentStartKey, ESplineCoordinateSpace::World);

				float ClosestDistance = TNumericLimits<float>::Max();
				FVector BestLocation = SubsegmentStart;

				for (int32 Step = 1; Step < NumSubdivisions; Step++)
				{
					const float SubsegmentEndKey = SelectedSegmentIndex + Step / static_cast<float>(NumSubdivisions);
					const FVector SubsegmentEnd = SplineComp->GetLocationAtSplineInputKey(SubsegmentEndKey, ESplineCoordinateSpace::World);

					FVector SplineClosest;
					FVector RayClosest;
					FMath::SegmentDistToSegmentSafe(SubsegmentStart, SubsegmentEnd, Click.GetOrigin(), Click.GetOrigin() + Click.GetDirection() * 50000.0f, SplineClosest, RayClosest);

					const float Distance = FVector::DistSquared(SplineClosest, RayClosest);
					if (Distance < ClosestDistance)
					{
						ClosestDistance = Distance;
						BestLocation = SplineClosest;
					}

					SubsegmentStartKey = SubsegmentEndKey;
					SubsegmentStart = SubsegmentEnd;
				}

				SelectedSplinePosition = BestLocation;

				return true;
			}
			else if (VisProxy->IsA(HVectorSplineTangentHandleProxy::StaticGetType()))
			{
				// Tangent handle clicked

				HVectorSplineTangentHandleProxy* KeyProxy = (HVectorSplineTangentHandleProxy*)VisProxy;

				// Note: don't change key selection when a tangent handle is clicked
				SelectedSegmentIndex = INDEX_NONE;
				SelectedTangentHandle = KeyProxy->KeyIndex;
				SelectedTangentHandleType = KeyProxy->bArriveTangent ? ESelectedTangentHandle::Arrive : ESelectedTangentHandle::Leave;

				CachedRotation = SplineComp->GetQuaternionAtSplinePoint(SelectedTangentHandle, ESplineCoordinateSpace::World);

				return true;
			}
		}
		else
		{
			SplineOwningActor = nullptr;
		}
	}

	return false;
}

UVectorSplineComponent* FVectorSplineComponentVisualizer::GetEditedSplineComponent() const
{
	return Cast<UVectorSplineComponent>(SplinePropertyPath.GetComponent());
}

bool FVectorSplineComponentVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	if (SplineComp != nullptr)
	{

		if (AVectorShapeActor* VectorShapeActor = Cast<AVectorShapeActor>(SplineComp->GetOwner()))
		{
			const float VectorShapeOffsetZ = SplineComp->OffsetZ;
			const FVector Offset = VectorShapeActor->GetActorUpVector() * VectorShapeOffsetZ;

			const FInterpCurveVector& Position = SplineComp->GetSplinePointsPosition();


			if (CurrentEditMode == ESplineEditMode::TransformMode)
			{
				OutLocation = FVector::ZeroVector;
				const int32 NumPoints = SplineComp->GetNumberOfSplinePoints();
				for (int32 PointIdx = 0; PointIdx < NumPoints; PointIdx++)
				{
					OutLocation += SplineComp->GetWorldLocationAtSplinePoint(PointIdx);
				}
				OutLocation /= static_cast<float>(NumPoints);

				OutLocation += Offset;
				return true;

			}
			else if (CurrentEditMode == ESplineEditMode::EditMode)
			{
				if (SelectedTangentHandle != INDEX_NONE)
				{
					// If tangent handle index is set, use that
					check(SelectedTangentHandle < Position.Points.Num());
					const auto& Point = Position.Points[SelectedTangentHandle];

					check(SelectedTangentHandleType != ESelectedTangentHandle::None);
					if (SelectedTangentHandleType == ESelectedTangentHandle::Leave)
					{
						OutLocation = SplineComp->GetComponentTransform().TransformPosition(Point.OutVal + Point.LeaveTangent);
					}
					else if (SelectedTangentHandleType == ESelectedTangentHandle::Arrive)
					{
						OutLocation = SplineComp->GetComponentTransform().TransformPosition(Point.OutVal - Point.ArriveTangent);
					}

					OutLocation += Offset;
					return true;
				}
				else if (LastKeyIndexSelected != INDEX_NONE)
				{
					// Otherwise use the last key index set
					check(LastKeyIndexSelected < Position.Points.Num());
					check(SelectedKeys.Contains(LastKeyIndexSelected));
					const auto& Point = Position.Points[LastKeyIndexSelected];
					OutLocation = SplineComp->GetComponentTransform().TransformPosition(Point.OutVal);

					OutLocation += Offset;
					return true;
				}
			}

		}
	}

	return false;
}

bool FVectorSplineComponentVisualizer::GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const
{
	if (ViewportClient->GetWidgetCoordSystemSpace() == COORD_Local || ViewportClient->GetWidgetMode() == UE::Widget::WM_Rotate)
	{
		UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
		if (SplineComp != nullptr)
		{
			OutMatrix = FRotationMatrix::Make(CachedRotation);
			return true;
		}
	}

	return false;
}

bool FVectorSplineComponentVisualizer::IsVisualizingArchetype() const
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	return (SplineComp && SplineComp->GetOwner() && FActorEditorUtils::IsAPreviewOrInactiveActor(SplineComp->GetOwner()));
}

bool FVectorSplineComponentVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	if (SplineComp != nullptr)
	{
		FInterpCurveVector& SplinePosition = SplineComp->GetSplinePointsPosition();
		FInterpCurveQuat& SplineRotation = SplineComp->GetSplinePointsRotation();
		FInterpCurveVector& SplineScale = SplineComp->GetSplinePointsScale();

		const int32 NumPoints = SplinePosition.Points.Num();

		if ((CurrentEditMode == ESplineEditMode::EditMode))
		{
			if (SelectedTangentHandle != INDEX_NONE)
			{
				// When tangent handles are manipulated...

				check(SelectedTangentHandle < NumPoints);

				if (!DeltaTranslate.IsZero())
				{
					check(SelectedTangentHandleType != ESelectedTangentHandle::None);

					SplineComp->Modify();

					FInterpCurvePoint<FVector>& EditedPoint = SplinePosition.Points[SelectedTangentHandle];
					if (EditedPoint.InterpMode == CIM_CurveBreak)
					{
						if (SelectedTangentHandleType == ESelectedTangentHandle::Leave)
						{
							EditedPoint.LeaveTangent += SplineComp->GetComponentTransform().InverseTransformVector(DeltaTranslate);
							EditedPoint.LeaveTangent.Z = 0.0f;
						}
						else
						{
							EditedPoint.ArriveTangent += SplineComp->GetComponentTransform().InverseTransformVector(-DeltaTranslate);
							EditedPoint.ArriveTangent.Z = 0.0f;
						}
					}
					else
					{
						const FVector Delta = (SelectedTangentHandleType == ESelectedTangentHandle::Leave) ? DeltaTranslate : -DeltaTranslate;
						FVector Tangent = EditedPoint.LeaveTangent + SplineComp->GetComponentTransform().InverseTransformVector(Delta);

						Tangent.Z = 0.0f;

						EditedPoint.LeaveTangent = Tangent;
						EditedPoint.ArriveTangent = Tangent;
					}
				}
			}
			else
			{
				// When spline keys are manipulated...

				check(LastKeyIndexSelected != INDEX_NONE);
				check(LastKeyIndexSelected < NumPoints);
				check(SelectedKeys.Num() > 0);

				SplineComp->Modify();

				if (ViewportClient->IsAltPressed() && bAllowDuplication)
				{
					DuplicateKey();

					// Don't duplicate again until we release LMB
					bAllowDuplication = false;
				}

				for (int32 SelectedKeyIndex : SelectedKeys)
				{
					FInterpCurvePoint<FVector>& EditedPoint = SplinePosition.Points[SelectedKeyIndex];
					FInterpCurvePoint<FQuat>& EditedRotPoint = SplineRotation.Points[SelectedKeyIndex];
					FInterpCurvePoint<FVector>& EditedScalePoint = SplineScale.Points[SelectedKeyIndex];

					if (!DeltaTranslate.IsZero())
					{
						// Find key position in world space
						const FVector CurrentWorldPos = SplineComp->GetComponentTransform().TransformPosition(EditedPoint.OutVal);
						// Move in world space
						const FVector NewWorldPos = CurrentWorldPos + DeltaTranslate;
						// Convert back to local space
						FVector NewLocalPos = SplineComp->GetComponentTransform().InverseTransformPosition(NewWorldPos);
						NewLocalPos.Z = 0.0f;
						EditedPoint.OutVal = NewLocalPos;
					}

					if (!DeltaRotate.IsZero())
					{
						// Set point tangent as user controlled
						EditedPoint.InterpMode = CIM_CurveUser;

						// Rotate tangent according to delta rotation
						FVector NewTangent = SplineComp->GetComponentTransform().GetRotation().RotateVector(EditedPoint.LeaveTangent); // convert local-space tangent vector to world-space
						NewTangent = DeltaRotate.RotateVector(NewTangent); // apply world-space delta rotation to world-space tangent
						NewTangent = SplineComp->GetComponentTransform().GetRotation().Inverse().RotateVector(NewTangent); // convert world-space tangent vector back into local-space
						NewTangent.Z = 0.0f;
						EditedPoint.LeaveTangent = NewTangent;
						EditedPoint.ArriveTangent = NewTangent;

						// Rotate spline rotation according to delta rotation
						FQuat NewRot = SplineComp->GetComponentTransform().GetRotation() * EditedRotPoint.OutVal; // convert local-space rotation to world-space
						NewRot = DeltaRotate.Quaternion() * NewRot; // apply world-space rotation
						NewRot = SplineComp->GetComponentTransform().GetRotation().Inverse() * NewRot; // convert world-space rotation to local-space
						NewRot.X = 0;
						NewRot.Z = 0;
						EditedRotPoint.OutVal = NewRot;
					}

					if (DeltaScale.X != 0.0f)
					{
						// Set point tangent as user controlled
						EditedPoint.InterpMode = CIM_CurveUser;

						FVector NewTangent = EditedPoint.LeaveTangent * (1.0f + DeltaScale.X);

						NewTangent.Z = 0.0f;

						EditedPoint.LeaveTangent = NewTangent;
						EditedPoint.ArriveTangent = NewTangent;
					}

					if (DeltaScale.Y != 0.0f)
					{
						// Scale in Y adjusts the scale spline
						EditedScalePoint.OutVal.Y *= (1.0f + DeltaScale.Y);
						EditedScalePoint.OutVal.Y = FMath::Max<float>(0.001f, EditedScalePoint.OutVal.Y);
					}

					if (DeltaScale.Z != 0.0f)
					{
						// Scale in Z adjusts the scale spline
						EditedScalePoint.OutVal.Z *= (1.0f + DeltaScale.Z);
					}
				}
			}
		}
		else if (CurrentEditMode == ESplineEditMode::TransformMode)
		{
		
			SplineComp->Modify();

			if (!DeltaTranslate.IsZero())
			{
				const FVector DeltaTranslateLocal = SplineComp->GetComponentTransform().InverseTransformVector(DeltaTranslate) ;

				for (int32 PointIdx = 0; PointIdx < NumPoints; PointIdx++)
				{
					FInterpCurvePoint<FVector>& PointPosition = SplinePosition.Points[PointIdx];
					PointPosition.OutVal += FVector(DeltaTranslateLocal.X, DeltaTranslateLocal.Y, 0.0f);
				}

				SplineComp->OffsetZ += DeltaTranslateLocal.Z;
			}
			if (!DeltaRotate.IsZero())
			{

				FVector CenterLocation = FVector::ZeroVector;
				for (int32 PointIdx = 0; PointIdx < NumPoints; PointIdx++)
				{
					CenterLocation += SplinePosition.Points[PointIdx].OutVal;
				}
				CenterLocation /= static_cast<float>(NumPoints);



				FVector CenterToPointVectorRotated;
				FVector NewTangent;
				for (int32 PointIdx = 0; PointIdx < NumPoints; PointIdx++)
				{
					FInterpCurvePoint<FVector>& PointPosition = SplinePosition.Points[PointIdx];
					CenterToPointVectorRotated = SplineComp->GetComponentTransform().GetRotation().RotateVector(PointPosition.OutVal - CenterLocation);
					CenterToPointVectorRotated = DeltaRotate.RotateVector(CenterToPointVectorRotated);
					PointPosition.OutVal = CenterLocation + CenterToPointVectorRotated;

					NewTangent = SplineComp->GetComponentTransform().GetRotation().RotateVector(PointPosition.LeaveTangent); // convert local-space tangent vector to world-space
					NewTangent = DeltaRotate.RotateVector(NewTangent); // apply world-space delta rotation to world-space tangent
					NewTangent = SplineComp->GetComponentTransform().GetRotation().Inverse().RotateVector(NewTangent); // convert world-space tangent vector back into local-space
					NewTangent.Z = 0.0f;
					PointPosition.LeaveTangent = NewTangent;

					NewTangent = SplineComp->GetComponentTransform().GetRotation().RotateVector(PointPosition.ArriveTangent); // convert local-space tangent vector to world-space
					NewTangent = DeltaRotate.RotateVector(NewTangent); // apply world-space delta rotation to world-space tangent
					NewTangent = SplineComp->GetComponentTransform().GetRotation().Inverse().RotateVector(NewTangent); // convert world-space tangent vector back into local-space
					NewTangent.Z = 0.0f;
					PointPosition.ArriveTangent = NewTangent;
				}
			}

			if (!DeltaScale.IsZero())
			{

				const float ShapeDeltaScale =  DeltaScale.X;

				const float WidthScale = DeltaScale.Y;

				FVector CenterLocation = FVector::ZeroVector;
				for (int32 PointIdx = 0; PointIdx < NumPoints; PointIdx++)
				{
					CenterLocation += SplinePosition.Points[PointIdx].OutVal;
				}
				CenterLocation /= static_cast<float>(NumPoints);

				bool bCanApplyScale = true;
				for (int32 PointIdx = 0; PointIdx < NumPoints; PointIdx++)
				{
					FInterpCurvePoint<FVector>& PointPosition = SplinePosition.Points[PointIdx];
					if (((PointPosition.OutVal - CenterLocation)* (1 + ShapeDeltaScale)).SizeSquared() < 100.0f)
					{
						bCanApplyScale = false;
						break;
					}

					FInterpCurvePoint<FVector>& EditedScalePoint = SplineScale.Points[PointIdx];
					// Scale in Y adjusts the scale spline
					EditedScalePoint.OutVal.Y *= (1.0f + DeltaScale.Y);
					EditedScalePoint.OutVal.Y = FMath::Max<float>(0.001f, EditedScalePoint.OutVal.Y);
				}

				if (bCanApplyScale)
				{
					for (int32 PointIdx = 0; PointIdx < NumPoints; PointIdx++)
					{
						FInterpCurvePoint<FVector>& PointPosition = SplinePosition.Points[PointIdx];
						const FVector CenterToPointVector = (PointPosition.OutVal - CenterLocation) * (1 + ShapeDeltaScale);
						PointPosition.OutVal = CenterLocation + CenterToPointVector;

						PointPosition.LeaveTangent *= (1 + ShapeDeltaScale);
						PointPosition.ArriveTangent *= (1 + ShapeDeltaScale);

						/*if (PointPosition.InterpMode != CIM_Linear)
							PointPosition.InterpMode = CIM_CurveAuto;*/
					}
				}
			}
		}

		if (!DeltaScale.IsZero() || !DeltaTranslate.IsZero() || !DeltaRotate.IsZero())
		{
			SplineComp->UpdateSpline();
			SplineComp->bSplineHasBeenEdited = true;

			NotifyPropertyModified(SplineComp, SplineCurvesProperty);
			return true;
		}
	}

	return false;
}

bool FVectorSplineComponentVisualizer::HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	bool bHandled = false;


	if (Key == EKeys::LeftMouseButton && Event == IE_Pressed)
	{
		if (SelectedSegment != FIntPoint::NoneValue)
		{
			SelectedTangentHandle = INDEX_NONE;
			SelectedTangentHandleType = ESelectedTangentHandle::None;
			ChangeSelectionState(SelectedSegment.X, false);
			SelectedSegmentIndex = SelectedSegment.X;
			OnAddKey();
			return true;
		}//
	}

	if (Key == EKeys::LeftMouseButton && Event == IE_Released)
	{
		UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
		if (SplineComp != nullptr)
		{
			// Recache widget rotation
			int32 Index = SelectedTangentHandle;
			if (Index == INDEX_NONE)
			{
				// If not set, fall back to last key index selected
				Index = LastKeyIndexSelected;
			}

			CachedRotation = SplineComp->GetQuaternionAtSplinePoint(Index, ESplineCoordinateSpace::World);
		}

		// Reset duplication flag on LMB release
		bAllowDuplication = true;
	}

	if (Event == IE_Pressed)
	{
		bHandled = SplineComponentVisualizerActions->ProcessCommandBindings(Key, FSlateApplication::Get().GetModifierKeys(), false);
	}

	return bHandled;
}

void FVectorSplineComponentVisualizer::EndEditing()
{
	SplineOwningActor = NULL;
	SplinePropertyPath.Reset();
	ChangeSelectionState(INDEX_NONE, false);
	SelectedSegmentIndex = INDEX_NONE;
	SelectedTangentHandle = INDEX_NONE;
	SelectedTangentHandleType = ESelectedTangentHandle::None;
	SelectedSegment = FIntPoint::NoneValue;
}

void FVectorSplineComponentVisualizer::ForceRefreshViewports() const
{
	GEditor->RedrawLevelEditingViewports(true);

	//HACK
	FViewport* ActiveViewport = GEditor->GetActiveViewport();
	if (ActiveViewport != nullptr && ActiveViewport->HasMouseCapture())
	{
		const int32	X = ActiveViewport->GetMouseX();
		const int32	Y = ActiveViewport->GetMouseY();
		ActiveViewport->SetMouse(X, Y + 1);
		ActiveViewport->UpdateMouseCursor(true);
		//ActiveViewport->LockMouseToViewport(true);
		ActiveViewport->SetMouse(X, Y);
	}
}

void FVectorSplineComponentVisualizer::OnDuplicateKey()
{
	const FScopedTransaction Transaction(LOCTEXT("DuplicateSplinePoint", "Duplicate Spline Point"));
	
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	DuplicateKey();

	SplineComp->UpdateSpline();
	SplineComp->bSplineHasBeenEdited = true;

	NotifyPropertyModified(SplineComp, SplineCurvesProperty);
}

void FVectorSplineComponentVisualizer::DuplicateKey()
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	check(SplineComp != nullptr);
	check(LastKeyIndexSelected != INDEX_NONE);
	check(SelectedKeys.Num() > 0);
	check(SelectedKeys.Contains(LastKeyIndexSelected));

	SplineComp->Modify();
	if (AActor* Owner = SplineComp->GetOwner())
	{
		Owner->Modify();
	}

	// Get a sorted list of all the selected indices, highest to lowest
	TArray<int32> SelectedKeysSorted;
	for (int32 SelectedKeyIndex : SelectedKeys)
	{
		SelectedKeysSorted.Add(SelectedKeyIndex);
	}
	SelectedKeysSorted.Sort([](int32 A, int32 B) { return A > B; });

	// Insert duplicates into the list, highest index first, so that the lower indices remain the same
	FInterpCurveVector& SplinePosition = SplineComp->SplineCurves.Position;
	FInterpCurveQuat& SplineRotation = SplineComp->SplineCurves.Rotation;
	FInterpCurveVector& SplineScale = SplineComp->SplineCurves.Scale;

	for (int32 SelectedKeyIndex : SelectedKeysSorted)
	{
		// Insert duplicates into arrays.
		// It's necessary to take a copy because copying existing array items by reference isn't allowed (the array may reallocate)
		SplinePosition.Points.Insert(FInterpCurvePoint<FVector>(SplinePosition.Points[SelectedKeyIndex]), SelectedKeyIndex);
		SplineRotation.Points.Insert(FInterpCurvePoint<FQuat>(SplineRotation.Points[SelectedKeyIndex]), SelectedKeyIndex);
		SplineScale.Points.Insert(FInterpCurvePoint<FVector>(SplineScale.Points[SelectedKeyIndex]), SelectedKeyIndex);

		// Adjust input keys of subsequent points
		for (int Index = SelectedKeyIndex + 1; Index < SplinePosition.Points.Num(); Index++)
		{
			SplinePosition.Points[Index].InVal += 1.0f;
			SplineRotation.Points[Index].InVal += 1.0f;
			SplineScale.Points[Index].InVal += 1.0f;
		}
	}

	// Repopulate the selected keys
	SelectedKeys.Empty();
	int32 Offset = SelectedKeysSorted.Num();
	for (int32 SelectedKeyIndex : SelectedKeysSorted)
	{
		SelectedKeys.Add(SelectedKeyIndex + Offset);

		if (LastKeyIndexSelected == SelectedKeyIndex)
		{
			LastKeyIndexSelected += Offset;
		}

		Offset--;
	}

	// Unset tangent handle selection
	SelectedTangentHandle = INDEX_NONE;
	SelectedTangentHandleType = ESelectedTangentHandle::None;

	ForceRefreshViewports();
}

bool FVectorSplineComponentVisualizer::CanAddKey() const
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	if (SplineComp == nullptr)
	{
		return false;
	}

	const int32 NumPoints = SplineComp->SplineCurves.Position.Points.Num();
	const int32 NumSegments = SplineComp->IsClosedLoop() ? NumPoints : NumPoints - 1;

	return (SelectedSegmentIndex != INDEX_NONE && SelectedSegmentIndex < NumSegments);
}




bool FVectorSplineComponentVisualizer::AreAllKeysSelected() const
{
	if (UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
	{
		return (SelectedKeys.Num() > 0 && SelectedKeys.Num() == SplineComp->GetNumberOfSplinePoints());
	}
	return false;
}

void FVectorSplineComponentVisualizer::OnAddKey()
{
	const FScopedTransaction Transaction(LOCTEXT("AddSplinePoint", "Add Spline Point"));
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	check(SplineComp != nullptr);
	check(LastKeyIndexSelected != INDEX_NONE);
	check(SelectedKeys.Num() > 0);
	check(SelectedKeys.Contains(LastKeyIndexSelected));
	check(SelectedTangentHandle == INDEX_NONE);
	check(SelectedTangentHandleType == ESelectedTangentHandle::None);

	SplineComp->Modify();
	if (AActor* Owner = SplineComp->GetOwner())
	{
		Owner->Modify();
	}

	FInterpCurveVector& SplinePosition = SplineComp->GetSplinePointsPosition();
	FInterpCurveQuat& SplineRotation = SplineComp->GetSplinePointsRotation();
	FInterpCurveVector& SplineScale = SplineComp->GetSplinePointsScale();

	FInterpCurvePoint<FVector> NewPoint(
		SelectedSegmentIndex,
		SplineComp->GetComponentTransform().InverseTransformPosition(SelectedSplinePosition),
		FVector::ZeroVector,
		FVector::ZeroVector,
		(SplinePosition.Points[SelectedSegmentIndex].InterpMode == CIM_CurveBreak || SplinePosition.Points[SelectedSegmentIndex].InterpMode == CIM_CurveUser) ? CIM_CurveAuto : SplinePosition.Points[SelectedSegmentIndex].InterpMode.GetValue());


	FInterpCurvePoint<FQuat> NewRotPoint(
		SelectedSegmentIndex,
		FQuat::Identity,
		FQuat::Identity,
		FQuat::Identity,
		CIM_CurveAuto);

	FInterpCurvePoint<FVector> NewScalePoint(
		SelectedSegmentIndex,
		FVector(1.0f),
		FVector::ZeroVector,
		FVector::ZeroVector,
		CIM_CurveAuto);

	SplinePosition.Points.Insert(NewPoint, SelectedSegmentIndex + 1);
	SplineRotation.Points.Insert(NewRotPoint, SelectedSegmentIndex + 1);
	SplineScale.Points.Insert(NewScalePoint, SelectedSegmentIndex + 1);

	// Adjust input keys of subsequent points
	for (int Index = SelectedSegmentIndex + 1; Index < SplinePosition.Points.Num(); Index++)
	{
		SplinePosition.Points[Index].InVal += 1.0f;
		SplineRotation.Points[Index].InVal += 1.0f;
		SplineScale.Points[Index].InVal += 1.0f;
	}

	// Set selection to 'next' key
	ChangeSelectionState(SelectedSegmentIndex + 1, false);
	SelectedSegmentIndex = INDEX_NONE;

	SplineComp->UpdateSpline();
	SplineComp->bSplineHasBeenEdited = true;

	NotifyPropertyModified(SplineComp, SplineCurvesProperty);

	CachedRotation = SplineComp->GetQuaternionAtSplinePoint(LastKeyIndexSelected, ESplineCoordinateSpace::World);

	ForceRefreshViewports();
}

void FVectorSplineComponentVisualizer::OnDeleteKey()
{

	if (UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
	{
		if (SplineComp->GetNumberOfSplinePoints() <= 3)
		{
			return;
		}

		const FScopedTransaction Transaction(LOCTEXT("DeleteSplinePoint", "Delete Spline Point"));
		check(SplineComp != nullptr);
		check(LastKeyIndexSelected != INDEX_NONE);
		check(SelectedKeys.Num() > 0);
		check(SelectedKeys.Contains(LastKeyIndexSelected));

		SplineComp->Modify();
		if (AActor* Owner = SplineComp->GetOwner())
		{
			Owner->Modify();
		}

		// Get a sorted list of all the selected indices, highest to lowest
		TArray<int32> SelectedKeysSorted;
		for (int32 SelectedKeyIndex : SelectedKeys)
		{
			SelectedKeysSorted.Add(SelectedKeyIndex);
		}
		SelectedKeysSorted.Sort([](int32 A, int32 B) { return A > B; });

		// Delete selected keys from list, highest index first
		FInterpCurveVector& SplinePosition = SplineComp->SplineCurves.Position;
		FInterpCurveQuat& SplineRotation = SplineComp->SplineCurves.Rotation;
		FInterpCurveVector& SplineScale = SplineComp->SplineCurves.Scale;

		for (int32 SelectedKeyIndex : SelectedKeysSorted)
		{
			SplinePosition.Points.RemoveAt(SelectedKeyIndex);
			SplineRotation.Points.RemoveAt(SelectedKeyIndex);
			SplineScale.Points.RemoveAt(SelectedKeyIndex);

			for (int Index = SelectedKeyIndex; Index < SplinePosition.Points.Num(); Index++)
			{
				SplinePosition.Points[Index].InVal -= 1.0f;
				SplineRotation.Points[Index].InVal -= 1.0f;
				SplineScale.Points[Index].InVal -= 1.0f;
			}
		}

		// Select first key
		ChangeSelectionState(0, false);
		SelectedSegmentIndex = INDEX_NONE;
		SelectedTangentHandle = INDEX_NONE;
		SelectedTangentHandleType = ESelectedTangentHandle::None;

		SplineComp->UpdateSpline();
		SplineComp->bSplineHasBeenEdited = true;

		NotifyPropertyModified(SplineComp, SplineCurvesProperty);

		CachedRotation = SplineComp->GetQuaternionAtSplinePoint(LastKeyIndexSelected, ESplineCoordinateSpace::World);

		ForceRefreshViewports();
	}
}

bool FVectorSplineComponentVisualizer::CanDeleteKey() const
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	return (SplineComp != nullptr &&
			SelectedKeys.Num() > 0 &&
			SelectedKeys.Num() != SplineComp->SplineCurves.Position.Points.Num() &&
			LastKeyIndexSelected != INDEX_NONE);
}


void FVectorSplineComponentVisualizer::OnSwitchMode()
{
	{
		ChangeSelectionState(INDEX_NONE, false);
		SelectedSegmentIndex = INDEX_NONE;
		SelectedTangentHandle = INDEX_NONE;
		SelectedTangentHandleType = ESelectedTangentHandle::None;
		SelectedSegment = FIntPoint::NoneValue;
	}

	if (CurrentEditMode == ESplineEditMode::EditMode)
	{
		CurrentEditMode = ESplineEditMode::TransformMode;
	}
	else if (CurrentEditMode == ESplineEditMode::TransformMode)
	{
		CurrentEditMode = ESplineEditMode::EditMode;
	}

	ForceRefreshViewports();
}

bool FVectorSplineComponentVisualizer::CanSwitchMode() const
{
	return (GetEditedSplineComponent() != nullptr);
}

void FVectorSplineComponentVisualizer::OnSelectAllKeys()
{
	if (UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
	{
		SelectedKeys.Empty();
		const int32 SplinePointsNum = SplineComp->GetNumberOfSplinePoints();
		
		SelectedSegmentIndex = INDEX_NONE;
		SelectedTangentHandle = INDEX_NONE;
		SelectedTangentHandleType = ESelectedTangentHandle::None;

		for (int32 KeyIndx = 0; KeyIndx < SplinePointsNum; KeyIndx++)
		{
			ChangeSelectionState(KeyIndx, KeyIndx != 0);
		}

		ForceRefreshViewports();
	}
}

bool FVectorSplineComponentVisualizer::CanSelectAllKeys() const
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	return (SplineComp != nullptr && SelectedKeys.Num() >= 1);
}

bool FVectorSplineComponentVisualizer::IsKeySelectionValid() const
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	return (SplineComp != nullptr &&
			SelectedKeys.Num() > 0 &&
			LastKeyIndexSelected != INDEX_NONE);
}

void FVectorSplineComponentVisualizer::OnSetKeyPositionType(EInterpCurveMode Mode)
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	if (SplineComp != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("SetSplinePointType", "Set Spline Point Type"));

		SplineComp->Modify();
		if (AActor* Owner = SplineComp->GetOwner())
		{
			Owner->Modify();
		}

		FInterpCurveVector& SplinePointsPosition = SplineComp->GetSplinePointsPosition();
		const bool bIsCCW = FPolygonTools::IsSplineWindingCCW(SplineComp);

		for (int32 SelectedKeyIndex : SelectedKeys)
		{
			TEnumAsByte<EInterpCurveMode>& InterpMode = SplinePointsPosition.Points[SelectedKeyIndex].InterpMode;

			if (InterpMode.GetValue() == CIM_CurveBreak  && Mode == CIM_CurveUser)
			{
				if (bIsCCW)
				{
					SplinePointsPosition.Points[SelectedKeyIndex].LeaveTangent = SplinePointsPosition.Points[SelectedKeyIndex].ArriveTangent;
				}
				else
				{
					SplinePointsPosition.Points[SelectedKeyIndex].ArriveTangent = SplinePointsPosition.Points[SelectedKeyIndex].LeaveTangent;
				}
				SplinePointsPosition.Points[SelectedKeyIndex].InterpMode = CIM_CurveUser;
			}
			else
			{
				SplinePointsPosition.Points[SelectedKeyIndex].InterpMode = Mode;
			}
		}

		
		NotifyPropertyModified(SplineComp, SplineCurvesProperty);
		ForceRefreshViewports();

	}
}

bool FVectorSplineComponentVisualizer::IsKeyPositionTypeSet(EInterpCurveMode Mode) const
{
	if (IsKeySelectionValid())
	{
		if (UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
		{
			for (int32 SelectedKeyIndex : SelectedKeys)
			{
				const auto& SelectedPoint = SplineComp->GetSplinePointsPosition().Points[SelectedKeyIndex];
				if (SelectedPoint.InterpMode == Mode)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void FVectorSplineComponentVisualizer::OnResetToDefault()
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	check(SplineComp != nullptr);

	const FScopedTransaction Transaction(LOCTEXT("ResetToDefault", "Reset to Default"));

	SplineComp->Modify();
	if (SplineOwningActor.IsValid())
	{
		SplineOwningActor.Get()->Modify();
	}

	SplineComp->bSplineHasBeenEdited = false;


	if (AVectorShapeActor* SplineOwner = Cast<AVectorShapeActor>(SplineComp->GetOwner()))
	{
		SplineComp->SplineColor = SplineOwner->SplineDefaultColor;

		FInterpCurveVector& SplineScale = SplineComp->GetSplinePointsScale();
		const int32 NumPoints = SplineScale.Points.Num();
		for (int32 PointIdx = 0; PointIdx < NumPoints; PointIdx++)
		{
			FInterpCurvePoint<FVector>& EditedScalePoint = SplineScale.Points[PointIdx];
			EditedScalePoint.OutVal = FVector::OneVector;
		}

		SplineComp->UpdateSpline();

		SplineOwner->RebuildVectorShapeMesh();
	}


	// Select first key
	ChangeSelectionState(0, false);
	SelectedSegmentIndex = INDEX_NONE;
	SelectedTangentHandle = INDEX_NONE;
	SelectedTangentHandleType = ESelectedTangentHandle::None;

	if (SplineOwningActor.IsValid())
	{
		SplineOwningActor.Get()->PostEditMove(false);
	}

	ForceRefreshViewports();
}

bool FVectorSplineComponentVisualizer::CanResetToDefault() const
{
	/*UVectorShapeSplineComponent* SplineComp = GetEditedSplineComponent();
	if(SplineComp != nullptr)
    {
        return SplineComp->SplineCurves != CastChecked<UVectorShapeSplineComponent>(SplineComp->GetArchetype())->SplineCurves;
    }
    else
    {
        return false;
    }*/

	return GetEditedSplineComponent() != nullptr;
}

void FVectorSplineComponentVisualizer::OnSetSplineType(bool bIsPolygon)
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	if (SplineComp != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("SwitchSplineType", "Switch Spline Type"));

		SplineComp->Modify();
		if (SplineOwningActor.IsValid())
		{
			SplineOwningActor.Get()->Modify();
		}

		if (!bIsPolygon)
		{
			SplineComp->SplineType = EVectorSplineType::Line;
		}
		else
		{
			SplineComp->SplineType = EVectorSplineType::Polygon;
		}

		SplineComp->PostEditChange();
	}
}

bool FVectorSplineComponentVisualizer::IsSplineTypeSet(bool bIsAdditive) const
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	if (SplineComp != nullptr)
	{
		/*return (bIsAdditive &&  SplineComp->SplineType == EVectorShapeSplineType::Additive) ||
			(!bIsAdditive &&  SplineComp->SplineType == EVectorShapeSplineType::Subtractive);*/

		return ((bIsAdditive ? 1 : 0) != (uint8)SplineComp->SplineType);
	}

	return false;
}

void FVectorSplineComponentVisualizer::OnDeleteSplineComponent()
{
	if (UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
	{
		if (AVectorShapeActor* SplineOwner = Cast<AVectorShapeActor>(SplineComp->GetOwner()))
		{
			VectorShapeEditorHelpers::RemoveSplineComponent(SplineComp);
			SplineOwner->RebuildVectorShapeMesh();
		}
	}
}

void FVectorSplineComponentVisualizer::OnDuplicateSplineComponent()
{
	if (UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
	{
		if (AVectorShapeActor* SplineOwner = Cast<AVectorShapeActor>(SplineComp->GetOwner()))
		{
			VectorShapeEditorHelpers::DuplicateSplineComponent(SplineComp);
			SplineOwner->RebuildVectorShapeMesh();
		}
	}
}

void FVectorSplineComponentVisualizer::OnToggleDrawShape()
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	if (SplineComp != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("OnToggleDrawShape", "Toggle Draw Shape"));

		SplineComp->Modify();
		if (SplineOwningActor.IsValid())
		{
			SplineOwningActor.Get()->Modify();
		}

		SplineComp->bDrawMesh = !SplineComp->bDrawMesh;

		SplineComp->PostEditChange();
		ForceRefreshViewports();

	}
}

bool FVectorSplineComponentVisualizer::CanToggleDrawShape() const
{
	if (const UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
	{
		if (const AVectorShapeActor* VectorShapeActor = Cast<AVectorShapeActor>(SplineComp->GetOwner()))
		{
			return VectorShapeActor->bDrawMesh;
		}
	}
	return false;
}

bool FVectorSplineComponentVisualizer::CanDrawShape() const
{
	if (const UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
	{
		if (const AVectorShapeActor* VectorShapeActor = Cast<AVectorShapeActor>(SplineComp->GetOwner()))
		{
			if (!VectorShapeActor->bDrawMesh)
			{
				return false;
			}

			return SplineComp->bDrawMesh;
		}
	}
	return false;
}

bool FVectorSplineComponentVisualizer::HasValidActor() const
{
	if (const UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
	{
		if (const AVectorShapeActor* VectorShapeActor = Cast<AVectorShapeActor>(SplineComp->GetOwner()))
		{
			return true;
		}
	}
	return false;
}

void FVectorSplineComponentVisualizer::OnApplyDefaultColor()
{
	if ( UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
	{
		if ( AVectorShapeActor* VectorShapeActor = Cast<AVectorShapeActor>(SplineComp->GetOwner()))
		{
			const FScopedTransaction Transaction(LOCTEXT("OnApplyDefaultColor", "Apply Default Color To Spline"));

			SplineComp->Modify();
			if (SplineOwningActor.IsValid())
			{
				SplineOwningActor.Get()->Modify();
			}

			SplineComp->SplineColor = VectorShapeActor->SplineDefaultColor;

			SplineComp->PostEditChange();
			ForceRefreshViewports();
		}
	}
}

TSharedPtr<SWidget> FVectorSplineComponentVisualizer::GenerateContextMenu() const
{
	FMenuBuilder MenuBuilder(true, SplineComponentVisualizerActions);
	{
		MenuBuilder.BeginSection("SplineMode", LOCTEXT("SplineMode", "Spline Mode"));
		{
			if (UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
			{
				const FText EditModeText = CurrentEditMode == ESplineEditMode::TransformMode ? LOCTEXT("EditMode", "Activate Edit Mode") : LOCTEXT("TransformMode", "Activate Transform Mode");
				MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().SwitchMode, NAME_None, EditModeText);
			}

		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("SplinePointEdit", LOCTEXT("SplinePoint", "Spline Point"));
		{
			if (SelectedSegmentIndex != INDEX_NONE)
			{
				MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().AddKey);
			}
			else if (LastKeyIndexSelected != INDEX_NONE)
			{
				MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().DeleteKey);
				MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().DuplicateKey);

				MenuBuilder.AddSubMenu(
					LOCTEXT("SplinePointType", "Spline Point Type"),
					LOCTEXT("KeyTypeTooltip", "Define the type of the spline point."),
					FNewMenuDelegate::CreateSP(this, &FVectorSplineComponentVisualizer::GenerateSplinePointTypeSubMenu),
					false,
					FSlateIcon(FVectorShapeEditorStyle::GetAppStyleSetName(), "VectorShapeEditor.PointCurveMode")
				);

				MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().SelectAllKeys);
			}
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("Spline", LOCTEXT("Spline", "Spline"));
		{
			if (UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
			{
				MenuBuilder.AddSubMenu(
					LOCTEXT("SplineType", "Spline Type"),
					LOCTEXT("SplineTypeTooltip", "Define the type of the spline."),
					FNewMenuDelegate::CreateSP(this, &FVectorSplineComponentVisualizer::GenerateSplineTypeSubMenu),
					false,
					FSlateIcon(FVectorShapeEditorStyle::GetAppStyleSetName(), "VectorShapeEditor.SplineType")
				);

				MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().DeleteSpline);
				MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().DuplicateSpline);
			}

		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("Rendering", LOCTEXT("Rendering", "Rendering"));
		{
			MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().DrawShape);
			MenuBuilder.AddWidget(CreateColorSelectionWidegt()->AsShared(), /*LOCTEXT("VertexColor", "Color"))*/FText::GetEmpty());
			MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().ApplyDefaultColor);
		}
		MenuBuilder.EndSection();


		MenuBuilder.BeginSection("Default", LOCTEXT("Default", "Default"));
		{
			MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().ResetToDefault);
		}
		MenuBuilder.EndSection();
	}

	TSharedPtr<SWidget> MenuWidget = MenuBuilder.MakeWidget();
	return MenuWidget;
}

void FVectorSplineComponentVisualizer::GenerateSplinePointTypeSubMenu(FMenuBuilder& MenuBuilder) const
{
	MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().SetKeyPositionToLinear, NAME_None, LOCTEXT("Linear", "Linear"));
	MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().SetKeyPositionToCurve, NAME_None, LOCTEXT("UserUser", "User"));
	MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().SetKeyPositionToCurveAuto, NAME_None, LOCTEXT("CurveAuto", "Auto"));
	MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().SetKeyPositionToCurveClamped, NAME_None, LOCTEXT("CurveAutoClamped", "Clamped"));
	MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().SetKeyPositionToCurveBreak, NAME_None, LOCTEXT("CurveBreak", "Break"));
}

void FVectorSplineComponentVisualizer::GenerateSplineTypeSubMenu(FMenuBuilder& MenuBuilder) const
{
	MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().SetSplineToPolygon);
	MenuBuilder.AddMenuEntry(FVectorSplineComponentVisualizerCommands::Get().SetSplineToLine);
}


TSharedPtr<SWidget> FVectorSplineComponentVisualizer::CreateColorSelectionWidegt() const
{
	return SNew(SColorBlock)
		.Color(this, &FVectorSplineComponentVisualizer::OnGetColorForColorBlock)
		.ShowBackgroundForAlpha(false)
		.AlphaDisplayMode(EColorBlockAlphaDisplayMode::Ignore)
		.OnMouseButtonDown(this, &FVectorSplineComponentVisualizer::OnMouseButtonDownColorBlock)
		.Size(FVector2D(35.0f, 12.0f));
}

FLinearColor FVectorSplineComponentVisualizer::OnGetColorForColorBlock() const
{
	if (UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
	{
		return SplineComp->SplineColor.ReinterpretAsLinear();
	}
	return FLinearColor::White;
}

FReply FVectorSplineComponentVisualizer::OnMouseButtonDownColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	if (UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
	{
		CreateColorPicker(true /*bUseAlpha*/);
		return FReply::Handled();
	}


	return FReply::Unhandled();
}

void FVectorSplineComponentVisualizer::CreateColorPicker(bool bUseAlpha) const
{

	if (UVectorSplineComponent* SplineComp = GetEditedSplineComponent())
	{
		const bool bRefreshOnlyOnOk = false;

		FColorPickerArgs PickerArgs;
		{
			PickerArgs.bUseAlpha = bUseAlpha;
			PickerArgs.bOnlyRefreshOnMouseUp = false;
			PickerArgs.bOnlyRefreshOnOk = bRefreshOnlyOnOk;
			PickerArgs.sRGBOverride = false;
			PickerArgs.DisplayGamma = TAttribute<float>::Create(TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma));
			PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &FVectorSplineComponentVisualizer::OnSetColorFromColorPicker);
			PickerArgs.OnColorPickerCancelled = FOnColorPickerCancelled::CreateSP(this, &FVectorSplineComponentVisualizer::OnColorPickerCancelled);
			PickerArgs.OnInteractivePickBegin = FSimpleDelegate::CreateSP(this, &FVectorSplineComponentVisualizer::OnColorPickerInteractiveBegin);
			PickerArgs.OnInteractivePickEnd = FSimpleDelegate::CreateSP(this, &FVectorSplineComponentVisualizer::OnColorPickerInteractiveEnd);
			PickerArgs.InitialColor = SplineComp->SplineColor;
			/*PickerArgs.ParentWidget = ColorPickerParentWidget;
			PickerArgs.OptionalOwningDetailsView = ColorPickerParentWidget;
			FWidgetPath ParentWidgetPath;
			if (FSlateApplication::Get().FindPathToWidget(ColorPickerParentWidget.ToSharedRef(), ParentWidgetPath))
			{
				PickerArgs.bOpenAsMenu = FSlateApplication::Get().FindMenuInWidgetPath(ParentWidgetPath).IsValid();
			}*/
		}

		OpenColorPicker(PickerArgs);
	}
}

void FVectorSplineComponentVisualizer::OnSetColorFromColorPicker(FLinearColor NewColor) const
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	if (SplineComp != nullptr)
	{
		//const FScopedTransaction Transaction(LOCTEXT("SetSplineColor", "Set Spline Color"));

		/*SplineComp->Modify();
		if (SplineOwningActor.IsValid())
		{
			SplineOwningActor.Get()->Modify();
		}
*/

		SplineComp->SplineColor = NewColor.ToFColor(true);

		SplineComp->PostEditChange();
		ForceRefreshViewports();
	}
}

void FVectorSplineComponentVisualizer::OnColorPickerCancelled(FLinearColor OriginalColor) const
{
	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	if (SplineComp != nullptr)
	{
		//const FScopedTransaction Transaction(LOCTEXT("SetSplineColor", "Set Spline Color"));

		SplineComp->Modify();
		if (SplineOwningActor.IsValid())
		{
			SplineOwningActor.Get()->Modify();
		}

		SplineComp->SplineColor = OriginalColor.ToFColor(true);

		SplineComp->PostEditChange();
		ForceRefreshViewports();
	}
}

void FVectorSplineComponentVisualizer::OnColorPickerInteractiveBegin() const
{
	GEditor->BeginTransaction(LOCTEXT("SetSplineColor", "Set Spline Color"));

	UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
	if (SplineComp != nullptr)
	{
		SplineComp->Modify();
		if (SplineOwningActor.IsValid())
		{
			SplineOwningActor.Get()->Modify();
		}
	}
}

void FVectorSplineComponentVisualizer::OnColorPickerInteractiveEnd() const
{
	//if (/*!bDontUpdateWhileEditing*/)
	{
		UVectorSplineComponent* SplineComp = GetEditedSplineComponent();
		if (SplineComp != nullptr)
		{
			SplineComp->PostEditChange();
			ForceRefreshViewports();
		}
	}

	GEditor->EndTransaction();
}

void FVectorSplineComponentVisualizer::CloseMenu() const
{
	FSlateApplication::Get().DismissAllMenus();
}

#undef LOCTEXT_NAMESPACE
