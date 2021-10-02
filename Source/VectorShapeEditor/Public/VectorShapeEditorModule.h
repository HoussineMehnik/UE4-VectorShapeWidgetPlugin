//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Async/AsyncWork.h"
#include "VectorShapeEditorTypes.h"





class AVectorShapeActor;
class IAssetTypeActions;


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
