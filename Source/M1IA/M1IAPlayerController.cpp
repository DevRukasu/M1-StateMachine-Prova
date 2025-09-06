// Copyright Epic Games, Inc. All Rights Reserved.
//aq em cima é tudo padrao da unreal, implementado ta ali em abaixo na linha 65~
#include "M1IAPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "M1IA.h"
#include "Widgets/Input/SVirtualJoystick.h"

#include "IA/IABase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"

void AM1IAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			MobileControlsWidget->AddToPlayerScreen(0);
		}
		else
		{
			UE_LOG(LogM1IA, Error, TEXT("Could not spawn mobile controls widget."));
		}
	}
}

void AM1IAPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}

	//inputs de debug
	InputComponent->BindAction("DamageBoss", IE_Pressed, this, &AM1IAPlayerController::DamageAI);
	InputComponent->BindAction("HealBoss", IE_Pressed, this, &AM1IAPlayerController::HealAI);

	//
	InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AM1IAPlayerController::DamageAI);
	InputComponent->BindKey(EKeys::Q, IE_Pressed, this, &AM1IAPlayerController::HealAI);
}

//MEU DEUS Q CU FAZER ISSO. ajuda a encontrar a IA mais proxima
AIABase* AM1IAPlayerController::FindClosestAIBase(float MaxSearchDist) const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(World, AIABase::StaticClass(), Found);
	if (Found.Num() == 0) return nullptr;

	const APawn* MyPawn = GetPawn();
	const FVector MyLoc = MyPawn ? MyPawn->GetActorLocation() : FVector::ZeroVector;

	float BestDistSq = MaxSearchDist * MaxSearchDist;
	AIABase* Best = nullptr;

	for (AActor* A : Found)
	{
		AIABase* AI = Cast<AIABase>(A);
		if (!AI) continue;

		const float DistSq = FVector::DistSquared(AI->GetActorLocation(), MyLoc);
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = AI;
		}
	}
	return Best;
}

void AM1IAPlayerController::PrintLine(const FString& Msg, FColor Color, float Time) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Time, Color, Msg);
	}
}
//Debug de dano e cura da IA
//Causa dano na IA no E
void AM1IAPlayerController::DamageAI()
{
    AIABase* TargetAI = FindClosestAIBase();
    if (!TargetAI)
    {
        PrintLine(TEXT("Sem IA no Level"), FColor::Red, 2.0f);
        return;
    }

    TargetAI->TakeDamage(10.0f, FDamageEvent(), this, this);

    const int32 HPct = FMath::Clamp(FMath::RoundToInt(TargetAI->GetHealthPercentage() * 100.f), 0, 100);
    PrintLine(FString::Printf(TEXT("AI tomou 10 de dano -> %d%% HP"), HPct), FColor::Red, 2.0f);
}

//Cura a IA quando aperta Q
void AM1IAPlayerController::HealAI()
{
    AIABase* TargetAI = FindClosestAIBase();
    if (!TargetAI)
    {
        PrintLine(TEXT("Sem IA no Level"), FColor::Red, 2.0f);
        return;
    }

    TargetAI->Heal(50.0f);

    const int32 HPct = FMath::Clamp(FMath::RoundToInt(TargetAI->GetHealthPercentage() * 100.f), 0, 100);
    PrintLine(FString::Printf(TEXT("AI curou 50 de vida -> %d%% HP"), HPct), FColor::Green, 2.0f);
}

