// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "M1IAGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AM1IAGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AM1IAGameMode();
};



