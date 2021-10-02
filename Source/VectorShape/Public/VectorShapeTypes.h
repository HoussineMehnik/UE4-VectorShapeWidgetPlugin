//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "Materials/Material.h"
#include "VectorShapeTypes.generated.h"


class AVectorShapeActor;
class USlateVectorShapeData;


//
DECLARE_LOG_CATEGORY_EXTERN(LogVectorShape, Log, All);

//
DECLARE_STATS_GROUP(TEXT("VectorMesh"), STATGROUP_VectorMesh, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("VectorWidget"), STATGROUP_VectorWidget, STATCAT_Advanced);


#ifdef WITH_EDITOR

struct VECTORSHAPE_API FVectorShapeEditorDelegates
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnCreateVectorMesh, AVectorShapeActor* /* VectorShapeActor */);
	static  FOnCreateVectorMesh OnCreateVectorMeshDelegate;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnVectorDataAssetChanged, USlateVectorShapeData* /* SlateVectorShapeData */);
	static  FOnVectorDataAssetChanged OnVectorDataAssetChangedDelegate;
};

#endif // WITH_EDITOR


UENUM()
enum class EVectorSplineType : uint8
{
	Polygon = 0,
	Line = 1
};


USTRUCT(BlueprintType)
struct VECTORSHAPE_API FVectorLineData
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VectorLineData)
		TArray<FVector2D> Points;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VectorLineData)
		 FLinearColor Tint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VectorLineData)
		bool bAntialias;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VectorLineData)
		float Thickness;


	FVectorLineData()
		: Tint(FLinearColor::White)
		, bAntialias(true)
		, Thickness(1.0f)
	{
		Points.Empty();
	}

};

class VECTORSHAPE_API FVectorShapeRuntimeHelpers
{
public:

	static const TCHAR* GetVectorMeshDefaultMaterialPath()
	{
		return TEXT("/VectorShape/Runtime/VectorMeshDefaultMaterial.VectorMeshDefaultMaterial");
	}

	static const TCHAR* GetVectorShapeWidgetDefaultMaterialPath()
	{
		return TEXT("/VectorShape/Runtime/VectorWidgetDefaultMaterial.VectorWidgetDefaultMaterial");
	}

	static UMaterialInterface* GetVectorMeshDefaultMaterial()
	{
		static UMaterialInterface* VectorMeshDefaultMaterial;
		if (VectorMeshDefaultMaterial == nullptr)
		{
			const FStringAssetReference ColoredMaterialName = FString(GetVectorMeshDefaultMaterialPath());
			VectorMeshDefaultMaterial = Cast<UMaterialInterface>(ColoredMaterialName.TryLoad());
		}
		return VectorMeshDefaultMaterial;
	}

	static UMaterialInterface* GetVectorShapeWidgetDefaultMaterial()
	{
		static UMaterialInterface* VectorShapeWidgetDefaultMaterial;
		if (VectorShapeWidgetDefaultMaterial == nullptr)
		{
			const FStringAssetReference ColoredMaterialName = FString(GetVectorShapeWidgetDefaultMaterialPath());
			VectorShapeWidgetDefaultMaterial = Cast<UMaterialInterface>(ColoredMaterialName.TryLoad());
		}

		return VectorShapeWidgetDefaultMaterial;
	}
};
