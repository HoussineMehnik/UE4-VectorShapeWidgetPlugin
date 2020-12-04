// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "SlateVectorShapeDataFactory.generated.h"

UCLASS(hidecategories = Object, collapsecategories, MinimalAPI)
class USlateVectorShapeDataFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	// ~ UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// ~ UFactory Interface	
};
