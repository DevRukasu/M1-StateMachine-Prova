#include "IA/IAChefao.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

AIAChefao::AIAChefao()
{
    PrimaryActorTick.bCanEverTick = true;

    // Retreat padrão do Chefe
    RetreatLocation = FVector(0.0f, 0.0f, 200.0f);
}


void AIAChefao::BeginPlay()
{
    Super::BeginPlay();
    bCanUseSpecialAttack = false;
}

void AIAChefao::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Verifica se deve ativar o Special Attack pela primeira vez
    if (!bCanUseSpecialAttack && ShouldActivateSpecialAttack())
    {
        bCanUseSpecialAttack = true;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Purple,
                TEXT("Especial Desbloqueado Vida abaixo de 50%"));
        }
    }

    // Se a vida voltou acima de 50%, desativa o Special Attack permanentemente
    else if (bCanUseSpecialAttack && !ShouldActivateSpecialAttack())
    {
        bCanUseSpecialAttack = false;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
                TEXT("Especial Bloqueado Vida acima de 50%"));
        }
    }

    // Debug do cooldown quando Special Attack está ativo mas em cooldown
    if (bCanUseSpecialAttack && GEngine)
    {
        if (!CanPerformBossSpecialAttack())
        {
            const float CurrentTime = GetWorld()->GetTimeSeconds();
            const float TimeUntilNextAttack = BossSpecialAttackCooldown - (CurrentTime - LastBossSpecialAttackTime);

            if (TimeUntilNextAttack > 0.0f)
            {
                GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Orange,
                    FString::Printf(TEXT("Special Attack Cooldown: %.1f s"), TimeUntilNextAttack));
            }
        }
        else
        {
            // Mostra se o jogador está no range
            bool bPlayerInRange = IsPlayerInSpecialAttackRange();
            FColor DebugColor = bPlayerInRange ? FColor::Green : FColor::Red;
            GEngine->AddOnScreenDebugMessage(-1, 0.0f, DebugColor,
                FString::Printf(TEXT("Jogador esta no range da Skill?: %s"),
                    bPlayerInRange ? TEXT("YES") : TEXT("NO")));
        }
    }
}

//Ataque Especial, se pode usar e como usa
void AIAChefao::PerformSpecialAttack()
{
    if (!CanPerformBossSpecialAttack()) return;

    const float CurrentTime = GetWorld()->GetTimeSeconds();
    LastBossSpecialAttackTime = CurrentTime;

    // Aplica dano em área
    ApplyAreaDamage(BossSpecialAttackDamage, BossSpecialAttackRadius);


}

bool AIAChefao::CanPerformBossSpecialAttack() const
{
    if (IsDead() || !bCanUseSpecialAttack) return false;

    // Verifica se o jogador está no range
    if (!IsPlayerInSpecialAttackRange()) return false;

    const float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastBossSpecialAttackTime) >= BossSpecialAttackCooldown;
}

bool AIAChefao::ShouldActivateSpecialAttack() const
{
    return GetHealthPercentage() <= SpecialAttackHealthThreshold;
}

bool AIAChefao::IsPlayerInSpecialAttackRange() const
{
    return IsPlayerInRange(BossSpecialAttackRadius);
}

void AIAChefao::ApplyAreaDamage(float Damage, float Radius)
{
    if (!GetWorld()) return;

    FVector Origin = GetActorLocation();

    // Encontra todos os atores em um raio
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        Origin,
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );

    int32 DamagedTargets = 0;

    if (bHasOverlap)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (AActor* HitActor = Result.GetActor())
            {
                // Verifica se é o jogador ou outro ator que pode receber dano
                if (HitActor->CanBeDamaged())
                {
                    ApplyDamageToTarget(HitActor, Damage);
                    DamagedTargets++;
                }
            }
        }
    }

    // Debug visual da área de efeito - APENAS uma sphere vermelha quando executado
    DrawDebugSphere(GetWorld(), Origin, Radius, 32, FColor::Red, false, 2.0f, 0, 3.0f);
}





// Healing especifico do Chefao
void AIAChefao::HandleHealing(float DeltaTime)
{
    if (IsDead()) return;

    if (CurrentHealth < MaxHealth)
    {
        // Cura gradual por segundo
        CurrentHealth += HealingRate * DeltaTime;

        if (CurrentHealth > MaxHealth)
        {
            CurrentHealth = MaxHealth;
        }

    }
}

bool AIAChefao::IsRecovering() const
{
    return CurrentHealth < MaxHealth;
}
