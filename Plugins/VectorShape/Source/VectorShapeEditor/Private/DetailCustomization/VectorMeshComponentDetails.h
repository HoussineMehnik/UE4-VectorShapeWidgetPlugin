// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

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
