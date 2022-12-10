//==========================================================================//
// Copyright Elhoussine Mehnik (ue4resources@gmail.com). All Rights Reserved.
//================== http://unrealengineresources.com/ =====================//

#pragma once

#include "CoreMinimal.h"
#include "Styling/ISlateStyle.h"



class FVectorShapeEditorStyle
{

private:
	static TSharedPtr< class FSlateStyleSet > StyleSet;

public:
	static void Initialize();
	static void Shutdown();
	static TSharedPtr<ISlateStyle> Get();
	static const ISlateStyle& GetRef();
	static FName GetAppStyleSetName();

private:
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);
};