//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#include "Factories/SlateVectorShapeDataFactory.h"

#include "VectorWidget/SlateVectorShapeData.h"


USlateVectorShapeDataFactory::USlateVectorShapeDataFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = USlateVectorShapeData::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* USlateVectorShapeDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class == USlateVectorShapeData::StaticClass());

	USlateVectorShapeData* SlateVectorArtData = NewObject<USlateVectorShapeData>(InParent, Name, Flags);

	return SlateVectorArtData;
}
