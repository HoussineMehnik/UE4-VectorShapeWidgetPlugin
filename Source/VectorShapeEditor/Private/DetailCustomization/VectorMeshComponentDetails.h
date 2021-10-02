//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class UVectorMeshComponent;
class AVectorShapeActor;

class FVectorMeshComponentDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder ) override;

	FReply ClickedOnSaveData();

	FReply ClickedOnAddNewSplineComponent(bool bIsAdditive);

	bool ConvertToStaticMeshEnabled() const;

	bool CanSaveData() const;

	FReply OnDeleteAllSpline();

	bool CanDeleteAllSplines() const;

	UVectorMeshComponent* GetSelectedVectorMeshComp() const;

	AVectorShapeActor* GetSelectedVectorShapeActor() const;

	TArray< TWeakObjectPtr<UObject> > SelectedObjectsList;
};
