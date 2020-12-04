
#include "VectorShapeEditorStyle.h"
#include "Styling/SlateStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "SlateOptMacros.h"
#include "Styling/SlateTypes.h"
#include "EditorStyleSet.h"



#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( FVectorShapeEditorStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )


FString FVectorShapeEditorStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString IconsDir = IPluginManager::Get().FindPlugin(TEXT("VectorShape"))->GetContentDir() / TEXT("Editor");
	return (IconsDir / RelativePath) + Extension;
}

TSharedPtr< FSlateStyleSet > FVectorShapeEditorStyle::StyleSet = NULL;
TSharedPtr< class ISlateStyle > FVectorShapeEditorStyle::Get() { return StyleSet; }
const ISlateStyle& FVectorShapeEditorStyle::GetRef() { return *(StyleSet.Get()); }

FName FVectorShapeEditorStyle::GetStyleSetName()
{
	return StyleSet->GetStyleSetName();
}


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FVectorShapeEditorStyle::Initialize()
{

	// Const icon & thumbnail sizes
	const FVector2D Icon16x16(16.0f, 16.f);
	const FVector2D Icon20x20(20.f, 20.f);
	const FVector2D Icon24x24(24.0f, 24.f);
	const FVector2D Icon28x28(28.0f, 28.f);
	const FVector2D Icon32x32(32.0f, 32.f);
	const FVector2D Icon40x40(40.f, 40.f);
	const FVector2D Icon64x64(64.f, 64.f);
	const FVector2D Icon128x128(128.f, 128.f);
	const FVector2D Icon512x512(512.f, 512.f);

	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	// Style Set Allocation
	StyleSet = MakeShareable(new FSlateStyleSet("VectorShapeEditorStyle"));
	FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("VectorShape"))->GetContentDir();
	StyleSet->SetContentRoot(ContentDir);

	// MiniMap Spline Menu Icons

	StyleSet->Set(TEXT("VectorShapeEditor.DuplicateKey"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_DuplicatePoint_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.DuplicateKey.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_DuplicatePoint_40x", Icon16x16));

	StyleSet->Set(TEXT("VectorShapeEditor.SelectAllKeys"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_SelectAllPoint_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.SelectAllKeys.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_SelectAllPoint_40x", Icon16x16));

	StyleSet->Set(TEXT("VectorShapeEditor.DeleteKey"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Delete_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.DeleteKey.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Delete_40x", Icon16x16));

	StyleSet->Set(TEXT("VectorShapeEditor.PointCurveMode"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_CurveModes_64x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.PointCurveMode.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_CurveModes_64x", Icon16x16));



	// Materials

	StyleSet->Set(TEXT("VectorShapeEditor.Materials"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_SetMaterials_64x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.Materials.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_SetMaterials_64x", Icon16x16));

	StyleSet->Set(TEXT("VectorShapeEditor.SetFloorMaterial"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Floor_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.SetFloorMaterial.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Floor_40x", Icon16x16));

	StyleSet->Set(TEXT("VectorShapeEditor.SetWallMaterial"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Wall_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.SetWallMaterial.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Wall_40x", Icon16x16));

	StyleSet->Set(TEXT("VectorShapeEditor.SetSideWallMaterial"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_SideWall_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.SetSideWallMaterial.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_SideWall_40x", Icon16x16));


	

	// Curve Mode
	StyleSet->Set(TEXT("VectorShapeEditor.SetKeyPositionToLinear"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_CurveLinear_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.SetKeyPositionToLinear.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_CurveLinear_40x", Icon16x16));

	StyleSet->Set(TEXT("VectorShapeEditor.SetKeyPositionToCurve"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_CurveUser_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.SetKeyPositionToCurve.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_CurveUser_40x", Icon16x16));

	StyleSet->Set(TEXT("VectorShapeEditor.SetKeyPositionToCurveAuto"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_CurveAuto_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.SetKeyPositionToCurveAuto.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_CurveAuto_40x", Icon16x16));

	StyleSet->Set(TEXT("VectorShapeEditor.SetKeyPositionToCurveClamped"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_CurveAutoClamped_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.SetKeyPositionToCurveClamped.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_CurveAutoClamped_40x", Icon16x16));

	StyleSet->Set(TEXT("VectorShapeEditor.SetKeyPositionToCurveBreak"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_CurveBreak_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.SetKeyPositionToCurveBreak.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_CurveBreak_40x", Icon16x16));

	//Spline Type

	StyleSet->Set(TEXT("VectorShapeEditor.SplineType"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Type", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.SplineType.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Type", Icon16x16));

	StyleSet->Set(TEXT("VectorShapeEditor.SetSplineToAdditive"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Add_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.SetSplineToAdditive.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Add_40x", Icon16x16));

	StyleSet->Set(TEXT("VectorShapeEditor.SetSplineToSubtractive"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Sub_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.SetSplineToSubtractive.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Sub_40x", Icon16x16));

	//Spline Visibility
	StyleSet->Set(TEXT("VectorShapeEditor.Visibility"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Visibility_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.Visibility.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Visibility_40x", Icon16x16));
	StyleSet->Set(TEXT("VectorShapeEditor.ShouldDrawFloor"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Floor_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.ShouldDrawFloor.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Floor_40x", Icon16x16));
	StyleSet->Set(TEXT("VectorShapeEditor.ShouldDrawDefaultWall"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Wall_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.ShouldDrawDefaultWall.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Wall_40x", Icon16x16));
	StyleSet->Set(TEXT("VectorShapeEditor.ShouldDrawSideWall"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_SideWall_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.ShouldDrawSideWall.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_SideWall_40x", Icon16x16));


	// Collision
	StyleSet->Set(TEXT("VectorShapeEditor.ShouldGenerateCollison"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_CollisionType_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.ShouldGenerateCollison.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_CollisionType_40x", Icon16x16));

	


	// Reset Properties & Settings
	StyleSet->Set(TEXT("VectorShapeEditor.ResetToDefault"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_ResetAll_32x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.ResetToDefault.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_ResetAll_32x", Icon16x16));

	StyleSet->Set(TEXT("VectorShapeEditor.DeleteSpline"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Delete_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.DeleteSpline.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Delete_40x", Icon16x16));


	StyleSet->Set(TEXT("VectorShapeEditor.DuplicateSpline"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Duplicate_40x", Icon40x40));
	StyleSet->Set(TEXT("VectorShapeEditor.DuplicateSpline.Small"), new IMAGE_BRUSH("Icons/icon_MiniMapSpline_Duplicate_40x", Icon16x16));

	
	

	StyleSet->Set(TEXT("Checker"), new IMAGE_BRUSH("Checker_256x", FVector2D(64, 64), FLinearColor(0.5f, 0.5f, 0.5f), ESlateBrushTileType::Both));

	

	// Class Thumbnails
	StyleSet->Set("ClassIcon.SlateVectorShapeData", new IMAGE_BRUSH("VectorAsset_64x", Icon16x16));
	StyleSet->Set("ClassThumbnail.SlateVectorShapeData", new IMAGE_BRUSH("VectorAsset_64x", Icon64x64));

	StyleSet->Set("ClassIcon.VectorShapeWidget", new IMAGE_BRUSH("VectorWidget_32x", Icon16x16));
	StyleSet->Set("ClassThumbnail.VectorShapeWidget", new IMAGE_BRUSH("VectorWidget_32x", Icon16x16));

	// Register Style Set
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
};

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef IMAGE_BRUSH


void FVectorShapeEditorStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

