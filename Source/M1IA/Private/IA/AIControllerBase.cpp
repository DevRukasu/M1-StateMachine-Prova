#include "IA/AIControllerBase.h"

#include "IA/IABase.h"
#include "IA/IAChefao.h"
#include "IA/IAGuardiao.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"

AAIControllerBase::AAIControllerBase()
{
    PrimaryActorTick.bCanEverTick = true;

    CustomPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("CustomPerceptionComponent"));
    SetPerceptionComponent(*CustomPerceptionComponent);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        //Configs do IAController para o PerceptionComponent, usei predefinido ja no .h, aproveita para ambos
        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

        CustomPerceptionComponent->ConfigureSense(*SightConfig);
        CustomPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }
}

void AAIControllerBase::BeginPlay()
{
    Super::BeginPlay();
}

void AAIControllerBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    AIABase* Base = Cast<AIABase>(GetPawn());
    AIAGuardiao* Guardian = Cast<AIAGuardiao>(Base);

    const float Dist = FVector::Dist(Base->GetActorLocation(), PlayerPawn->GetActorLocation());

    //Exclusivo do guardiao
    if (Guardian)
    {
        // Retreat é prio do guardiao
        if (Guardian->bHasRetreatStarted && CurrentState == EAIState::Retreat)
        {
            SetTargetActor(nullptr);
            UpdateStateMachine(DeltaTime);
            return;
        }

        // Se não está em retreat, aplica lógica normal de detecção
        if (!Guardian->bHasRetreatStarted)
        {
            if (Dist <= Guardian->AlertDistance)
            {
                SetTargetActor(PlayerPawn);
            }
            else
            {
                SetTargetActor(nullptr);
            }
        }
    }
    else
    {
        if (Dist <= SightRadius)
        {
            SetTargetActor(PlayerPawn);
        }
        else
        {
            SetTargetActor(nullptr);
        }
    }

    UpdateStateMachine(DeltaTime);
}





//State Machine, Update
void AAIControllerBase::UpdateStateMachine(float DeltaTime)
{
    //Identifica quem esta sendo controlado no level
    AIABase* Base = Cast<AIABase>(GetPawn());
    AIAChefao* Boss = Cast<AIAChefao>(Base);
    AIAGuardiao* Guardian = Cast<AIAGuardiao>(Base);

    if (!Base) return;

    StateTimer += DeltaTime;
    const float DistanceToPlayer = Base->GetDistanceToTarget(); // único cálculo de distância que esta do player

    //Maior prioridade para o Guardiao
    if (Guardian)
    {
        // Verificar se deve iniciar Retreat (prioridade máxima)
        if (Guardian->ShouldStartRetreat() && !Guardian->bHasRetreatStarted)
        {
            Guardian->StartRetreat();
            SetCurrentState(EAIState::Retreat);
            SetTargetActor(nullptr); // Remove target para garantir que não interfira
            ClearFocus(EAIFocusPriority::Gameplay);
            return;
        }

        // Se já está em Retreat, IGNORA TUDO e continua no retreat
        if (Guardian->bHasRetreatStarted && CurrentState == EAIState::Retreat)
        {
            Guardian->HandleRetreatMovement();
            return;
        }
    }

    //Especifico para Guardiao
    if (Guardian)
    {
        // Se não há target, vai para Patrol
        if (!TargetActor)
        {
            if (CurrentState != EAIState::Patrol)
            {
                SetCurrentState(EAIState::Patrol);
                ClearFocus(EAIFocusPriority::Gameplay);
            }
        }


        if (Guardian->IsPlayerInAlertRange() && DistanceToPlayer > Base->GetChaseDistance())
        {
            if (CurrentState != EAIState::Alert)
            {
                SetCurrentState(EAIState::Alert);
                Guardian->AlertTimer = 0.0f;
            }

            Guardian->HandleAlertBehavior();

          
            if (Guardian->AlertTimer >= Guardian->AlertMinTime &&
                DistanceToPlayer <= Base->GetChaseDistance())
            {
                SetCurrentState(EAIState::Chase);
            }
            return;
        }

        // Se jogador saiu do alcance de Alert, volta para Patrol mesmo ele nao andando
        if (!Guardian->IsPlayerInAlertRange() && CurrentState == EAIState::Alert)
        {
            SetTargetActor(nullptr);
            SetCurrentState(EAIState::Patrol);
            ClearFocus(EAIFocusPriority::Gameplay);
            return;
        }

        // Se jogador está na Chase distance, vai para Chase
        if (DistanceToPlayer <= Base->GetChaseDistance() && TargetActor)
        {
            if (CurrentState != EAIState::Chase && CurrentState != EAIState::Attack &&
                CurrentState != EAIState::RangedAttack && CurrentState != EAIState::SpecialAttack)
            {
                SetCurrentState(EAIState::Chase);
            }
        }
    }

    if (!TargetActor && !Guardian)
    {
        if (CurrentState != EAIState::Idle)
        {
            SetCurrentState(EAIState::Idle);
            ClearFocus(EAIFocusPriority::Gameplay);
        }
        return;
    }

    constexpr float AttackEnterDelay = 0.5f;





    switch (CurrentState)
    {
    case EAIState::Idle:
        if (Guardian)
        {
            // Guardian fica realmente em Idle (parado)
            ClearFocus(EAIFocusPriority::Gameplay);
            StopMovement();
        }
        else
        {
            // Outros inimigos continuam indo para Chase
            SetCurrentState(EAIState::Chase);
        }
        break;





    case EAIState::Patrol:
        if (Guardian)
        {
            if (TargetActor)
            {
                if (Guardian->IsPlayerInAlertRange() && DistanceToPlayer > Base->GetChaseDistance())
                {
                    SetCurrentState(EAIState::Alert);
                    return;
                }
            }

            Guardian->SetMovementSpeed(Guardian->PatrolSpeed);
        }
        break;





    case EAIState::Alert:
        if (!Guardian)
        {
            SetCurrentState(EAIState::Chase);
        }

        break;





    case EAIState::Chase:
        if (Boss && !Boss->bHasHealedOnce && Base->GetHealthPercentage() <= 0.2f)
        {
            SetCurrentState(EAIState::Healing);
            return;
        }

        if (Guardian && Guardian->CanPerformGuardianSpecialAttack() && StateTimer >= AttackEnterDelay)
        {
            SetCurrentState(EAIState::SpecialAttack);
            return;
        }

        if (Boss && Boss->CanPerformBossSpecialAttack() && StateTimer >= AttackEnterDelay)
        {
            SetCurrentState(EAIState::SpecialAttack);
            return;
        }

        SetFocus(TargetActor);
        MoveToActor(TargetActor, 5.f);
        Base->SetMovementSpeed(Base->ChaseSpeed);

        if (DistanceToPlayer <= Base->GetMeleeAttackRange() && StateTimer >= AttackEnterDelay)
        {
            SetCurrentState(EAIState::Attack);
        }
        else if (DistanceToPlayer <= Base->GetRangedAttackRange() && StateTimer >= AttackEnterDelay)
        {
            SetCurrentState(EAIState::RangedAttack);
        }
        break;





    case EAIState::Attack:
        if (Boss && Boss->CanPerformBossSpecialAttack())
        {
            SetCurrentState(EAIState::SpecialAttack);
            return;
        }
        if (Guardian && Guardian->CanPerformGuardianSpecialAttack())
        {
            SetCurrentState(EAIState::SpecialAttack);
            return;
        }

        SetFocus(TargetActor);
        StopMovement();

        if (Base->CanAttack() && StateTimer >= 2.f)
        {
            Base->PerformMeleeAttack();
            UGameplayStatics::ApplyDamage(TargetActor, 20.f, this, Base, UDamageType::StaticClass());
            StateTimer = 0.f;
        }

        if (DistanceToPlayer > Base->GetMeleeAttackRange() && DistanceToPlayer <= Base->GetRangedAttackRange())
        {
            SetCurrentState(EAIState::RangedAttack);
        }
        else if (DistanceToPlayer > Base->GetRangedAttackRange())
        {
            SetCurrentState(EAIState::Chase);
        }
        break;





    case EAIState::RangedAttack:
        if (Boss && Boss->CanPerformBossSpecialAttack())
        {
            SetCurrentState(EAIState::SpecialAttack);
            return;
        }
        if (Guardian && Guardian->CanPerformGuardianSpecialAttack())
        {
            SetCurrentState(EAIState::SpecialAttack);
            return;
        }

        SetFocus(TargetActor);
        StopMovement();

        if (Base->CanAttack() && StateTimer >= 2.f)
        {
            Base->PerformRangedAttack();
            UGameplayStatics::ApplyDamage(TargetActor, 10.f, this, Base, UDamageType::StaticClass());
            StateTimer = 0.f;
        }

        if (DistanceToPlayer <= Base->GetMeleeAttackRange())
        {
            SetCurrentState(EAIState::Attack);
        }
        else if (DistanceToPlayer > Base->GetRangedAttackRange())
        {
            SetCurrentState(EAIState::Chase);
        }
        break;





    case EAIState::SpecialAttack:
        SetFocus(TargetActor);
        StopMovement();

        if (Guardian && StateTimer >= 0.5f && StateTimer < 0.6f)
        {
            Guardian->PerformSpecialAttack();
            // Chamar reforços (debug na tela apenas)
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(1001, 5.0f, FColor::Blue,
                    TEXT("Habilidae Especial - Chamando Reforços (ou algo assim)"));
            }
        }
        else if (Boss && StateTimer >= 0.5f && StateTimer < 0.6f)
        {
            Base->PerformSpecialAttack();
        }

        if (StateTimer >= 1.5f)
        {
            SetCurrentState(EAIState::Chase);
        }
        break;





    case EAIState::Retreat:
        if (Guardian)
        {
            ClearFocus(EAIFocusPriority::Gameplay);
            Guardian->HandleRetreatMovement();

            // Quando terminar o retreat, volta para Idle
            if (Guardian->bHasFinishedRetreat)
            {
                SetCurrentState(EAIState::Idle);
                Guardian->SetMovementSpeed(Guardian->WalkSpeed);
                return;
            }
        }
        else
        {
            SetCurrentState(EAIState::Chase);
        }
        break;





    case EAIState::Healing:
        if (Boss)
        {
            ClearFocus(EAIFocusPriority::Gameplay);

            if (TargetActor && DistanceToPlayer >= 0.0f && DistanceToPlayer < 500.f)
            {
                SetCurrentState(EAIState::Chase);
                return;
            }

            const FVector RetreatPos = Base->RetreatLocation;
            const float DistToRetreat = FVector::Dist(Base->GetActorLocation(), RetreatPos);
            if (DistToRetreat > 150.f)
            {
                MoveToLocation(RetreatPos);
            }
            else
            {
                StopMovement();
                Boss->HandleHealing(DeltaTime);

                if (!Boss->IsRecovering())
                {
                    Boss->bHasHealedOnce = true;
                    if (TargetActor)
                    {
                        const float DistToPlayer2 = FVector::Dist(Base->GetActorLocation(), TargetActor->GetActorLocation());
                        if (DistToPlayer2 <= SightRadius)
                        {
                            SetCurrentState(EAIState::Chase);
                        }
                        else
                        {
                            SetTargetActor(nullptr);
                            SetCurrentState(EAIState::Idle);
                            ClearFocus(EAIFocusPriority::Gameplay);
                        }
                    }
                    else
                    {
                        SetCurrentState(EAIState::Idle);
                        ClearFocus(EAIFocusPriority::Gameplay);
                    }
                }
            }
        }
        break;

    default:
        break;
    }
}

void AAIControllerBase::SetCurrentState(EAIState NewState)
{
    CurrentState = NewState;
    StateTimer = 0.f;
}

EAIState AAIControllerBase::GetCurrentState() const
{
    return CurrentState;
}

void AAIControllerBase::SetTargetActor(AActor* NewTarget)
{
    TargetActor = NewTarget;
}

AActor* AAIControllerBase::GetTargetActor() const
{
    return TargetActor;
}
