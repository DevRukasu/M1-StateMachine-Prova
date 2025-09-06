#include "IA/IAGuardiao.h"
#include "IA/AIControllerBase.h"
#include "Portao.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

AIAGuardiao::AIAGuardiao()
{
    PrimaryActorTick.bCanEverTick = true;

    // Configurações específicas do Guardião, sao herdadas do IABase
    MaxHealth = 120.0f;
    CurrentHealth = MaxHealth;
    WalkSpeed = 250.0f;
    ChaseSpeed = 450.0f;
    PatrolSpeed = 150.0f;

    MeleeAttackDamage = 25.0f;
    RangedAttackDamage = 15.0f;
    MeleeAttackRange = 300.0f;
    RangedAttackRange = 800.0f;

    // Valores de Distancia que ve algo
    AlertDistance = 2200.0f;
    ChaseDistance = 1500.0f;

    // Retreat padrão do Guardião
    RetreatLocation = FVector(0.0f, 1000.0f, 0.0f);
}

void AIAGuardiao::BeginPlay()
{
    Super::BeginPlay();

    // Procurar pelo portão no mundo se não foi atribuído
    if (!GateToOpen)
    {
        TArray<AActor*> FoundGates;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APortao::StaticClass(), FoundGates);
        if (FoundGates.Num() > 0)
        {
            GateToOpen = Cast<APortao>(FoundGates[0]);
        }
    }
}


void AIAGuardiao::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (IsDead()) return;

    // Verificar se deve ativar Special Attack (Call Reinforcements) - apenas se não está em retreat
    if (!bHasCalledReinforcements && !bHasRetreatStarted && ShouldActivateSpecialAttack())
    {
        bHasCalledReinforcements = true;
        PerformSpecialAttack();
    }

    // Verificar se deve iniciar Retreat - prioridade absoluta
    if (!bHasRetreatStarted && ShouldStartRetreat())
    {
        StartRetreat();
    }

    // Debug info específico do Guardião
    if (GEngine)
    {
        AAIControllerBase* AIController = Cast<AAIControllerBase>(GetController());
        if (AIController)
        {
            FString ExtraInfo;

            if (bHasCalledReinforcements)
                ExtraInfo += " | Reinforcements Called";


            if (AIController->GetCurrentState() == EAIState::Alert)
            {
                ExtraInfo += FString::Printf(TEXT(" | Alert Timer: %.1f"), AlertTimer);
            }
        }
    }
}




//Especial do Guardiao, simples
void AIAGuardiao::PerformSpecialAttack()
{
    if (!CanPerformGuardianSpecialAttack()) return;

    LastSpecialAttackTime = GetWorld()->GetTimeSeconds();



    bHasCalledReinforcements = true;

    UE_LOG(LogTemp, Warning, TEXT("Guardian %s called for reinforcements!"), *GetName());
}



bool AIAGuardiao::CanPerformGuardianSpecialAttack() const
{
    if (IsDead() || bHasCalledReinforcements || bHasRetreatStarted) return false;
    return ShouldActivateSpecialAttack();
}

bool AIAGuardiao::ShouldActivateSpecialAttack() const
{
    return GetHealthPercentage() <= SpecialAttackHealthThreshold; // 40% da vida, threshold do Call Reinfor
}

bool AIAGuardiao::ShouldStartRetreat() const
{
    return GetHealthPercentage() <= RetreatHealthThreshold && !bHasFinishedRetreat;
}

bool AIAGuardiao::IsPlayerInAlertRange() const
{
    return IsPlayerInRange(AlertDistance); // consegue ver o jogador em 20m? 
}


//Sistema de Alerta, exclusivo do guardiao, simples mas funcional
void AIAGuardiao::HandleAlertBehavior()
{
    AActor* Player = GetPlayerTarget();
    if (!Player) return;

    if (AAIControllerBase* AIController = Cast<AAIControllerBase>(GetController()))
    {
        AIController->SetFocus(Player); // foca no player a visao, sempre que tiver isso
        AIController->StopMovement();//Parar o movimento dela, mais facil usar isso doq setar movespeed para 0
    }

    AlertTimer += GetWorld()->GetDeltaSeconds();//timer de quanto tempo ta olhando. serve so para o debug

    //debug abaixo
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow,
            FString::Printf(TEXT("Guardiao em Alerta: olhando o jogador (%.1fs) - Distancia: %.0f"),
                AlertTimer, GetDistanceToTarget()));
    }
}

//Retreat (corre para dentro do portao)
void AIAGuardiao::StartRetreat()
{
    if (bHasRetreatStarted) return;

    bHasRetreatStarted = true;

    // Abrir portão
    OpenGate();
}

void AIAGuardiao::OpenGate()
{
    if (GateToOpen && !bGateOpened)
    {
        GateToOpen->OpenGate();
        bGateOpened = true;
    }
}

void AIAGuardiao::HandleRetreatMovement()
{
    float DistanceToRetreat = FVector::Dist(GetActorLocation(), RetreatLocation);

    if (DistanceToRetreat > 150.0f && !bHasFinishedRetreat)
    {
        if (AAIControllerBase* AIController = Cast<AAIControllerBase>(GetController()))
        {
            AIController->MoveToLocation(RetreatLocation);
            SetMovementSpeed(ChaseSpeed);
        }
    }
    else
    {
        if (AAIControllerBase* AIController = Cast<AAIControllerBase>(GetController()))
        {
            AIController->StopMovement();
            SetMovementSpeed(0.0f);
        }

        if (!bHasFinishedRetreat)
        {
            bHasFinishedRetreat = true;
        }

        static float RetreatFinishedTimer = 0.0f;
        RetreatFinishedTimer += GetWorld()->GetDeltaSeconds();

    }
}
