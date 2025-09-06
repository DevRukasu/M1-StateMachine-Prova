#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnumStateMachine.h"
#include "AIControllerBase.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class M1IA_API AAIControllerBase : public AAIController
{
    GENERATED_BODY()

public:
    AAIControllerBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
    UAIPerceptionComponent* CustomPerceptionComponent;

    UPROPERTY()
    UAISenseConfig_Sight* SightConfig;

    // === Ajustáveis no editor ===
    UPROPERTY(EditAnywhere, Category = "AI|Perception")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "AI|Perception")
    float LoseSightRadius = 1600.0f;

    UPROPERTY(EditAnywhere, Category = "AI|Perception")
    float PeripheralVisionAngle = 90.0f;

    // === State Machine ===
    UPROPERTY(VisibleAnywhere, Category = "AI|State")
    EAIState CurrentState;

    UPROPERTY(VisibleAnywhere, Category = "AI|State")
    float StateTimer = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "AI|Target")
    AActor* TargetActor = nullptr;

public:
    void UpdateStateMachine(float DeltaTime);

    void SetCurrentState(EAIState NewState);
    EAIState GetCurrentState() const;

    void SetTargetActor(AActor* NewTarget);
    AActor* GetTargetActor() const;
};
