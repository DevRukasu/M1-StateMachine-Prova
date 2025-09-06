// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "M1IAPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class AIABase;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings and debug controls
 */
UCLASS()
class AM1IAPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup + debug binds */
	virtual void SetupInputComponent() override;

	// ===== Debug: controle de vida da IA (qualquer classe que herde de AIABase) =====
	void DamageAI();
	void HealAI();

private:
	/** Retorna a IA mais próxima do jogador (qualquer classe que herde de AIABase) */
	AIABase* FindClosestAIBase(float MaxSearchDist = 100000.f) const;

	/** Helper para printar uma linha no HUD */
	void PrintLine(const FString& Msg, FColor Color = FColor::Green, float Time = 2.0f) const;
};
