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
class USlateVectorShapeData;

class FSlateVectorShapeDataDetails : public IDetailCustomization
{

private:
	FSlateVectorShapeDataDetails();

public:
	~FSlateVectorShapeDataDetails();

	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder ) override;

private:

	FText GetVerticesNumText() const;

	FText GetIndicesNumText() const;

	FText GetTrianglesNumText() const;

	FText GetVectorShapeSizeText() const;

protected:

	void RefreshDetails(USlateVectorShapeData* SlateVectorShapeData);



private:

	IDetailLayoutBuilder * DetailBuilderPtr;

	TWeakObjectPtr<USlateVectorShapeData> SelectedVectorShapeData;
};
