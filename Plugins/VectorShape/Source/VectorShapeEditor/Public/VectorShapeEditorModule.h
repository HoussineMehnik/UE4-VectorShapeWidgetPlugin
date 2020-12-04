// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Async/AsyncWork.h"
#include "VectorShapeEditorTypes.h"





class AVectorShapeActor;



class  FVectorShapeEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;


private:

	void OnCreateVectorShapeMesh(AVectorShapeActor* VectorShapeActor);

	
};
