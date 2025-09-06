#include "IA/IABase.h"
#include "IA/AIControllerBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

AIABase::AIABase()
{
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AAIControllerBase::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	CurrentHealth = MaxHealth;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

		// IA gira na direção do movimento
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	}
}

void AIABase::BeginPlay()
{
	Super::BeginPlay();
}

void AIABase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;

	AAIControllerBase* AIController = Cast<AAIControllerBase>(GetController());
	if (AIController && AIController->GetCurrentState() == EAIState::Healing)
	{
		HandleHealing(DeltaTime);
	}

	// === DEBUG ON-SCREEN ===
	if (GEngine && AIController)
	{
		FString StateText;
		switch (AIController->GetCurrentState())
		{
		case EAIState::Idle: StateText = "IDLE"; break;
		case EAIState::Patrol: StateText = "PATROL"; break;
		case EAIState::Alert: StateText = "ALERT"; break;
		case EAIState::Chase: StateText = "CHASE"; break;
		case EAIState::Attack: StateText = "ATTACK"; break;
		case EAIState::RangedAttack: StateText = "RANGED ATTACK"; break;
		case EAIState::SpecialAttack: StateText = "SPECIAL ATTACK"; break;
		case EAIState::Retreat: StateText = "RETREAT"; break;
		case EAIState::Healing: StateText = "HEALING"; break;
		case EAIState::Dead: StateText = "DEAD"; break;
		default: StateText = "UNKNOWN"; break;
		}

		float CurrentSpeed = GetCharacterMovement() ? GetCharacterMovement()->Velocity.Size() : 0.0f;

		FString DebugText = FString::Printf(
			TEXT("Estado: %s | Speed: %.1f | Health: %.0f/%.0f"),
			*StateText,
			CurrentSpeed,
			CurrentHealth,
			MaxHealth
		);

		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, DebugText);
	}
}



// --- Vida
float AIABase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDead()) return 0.0f;

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

	if (IsDead())
	{
		HandleDeath();
	}

	UE_LOG(LogTemp, Log, TEXT("%s took %f damage. Health: %f/%f"), *GetName(), DamageAmount, CurrentHealth, MaxHealth);
	return DamageAmount;
}


void AIABase::Heal(float HealAmount)
{
	if (IsDead()) return;

	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
}


// --- Combate
bool AIABase::CanAttack() const
{
	if (IsDead()) return false;
	const float Now = GetWorld()->GetTimeSeconds();
	return (Now - LastAttackTime) >= AttackCooldown;
}

bool AIABase::CanPerformSpecialAttack() const
{
	if (IsDead()) return false;
	const float Now = GetWorld()->GetTimeSeconds();
	return (Now - LastSpecialAttackTime) >= SpecialAttackCooldown;
}

void AIABase::PerformMeleeAttack()
{
	if (!CanAttack()) return;
	LastAttackTime = GetWorld()->GetTimeSeconds();

	AActor* Player = GetPlayerTarget();
	if (Player && IsPlayerInRange(MeleeAttackRange))
	{
		ApplyDamageToTarget(Player, MeleeAttackDamage);
	}
}


void AIABase::PerformRangedAttack()
{
	if (!CanAttack()) return;
	LastAttackTime = GetWorld()->GetTimeSeconds();

	AActor* Player = GetPlayerTarget();
	if (Player && IsPlayerInRange(RangedAttackRange) && !IsPlayerInRange(MeleeAttackRange))
	{
		ApplyDamageToTarget(Player, RangedAttackDamage);
	}
}


void AIABase::PerformSpecialAttack()
{
	if (!CanPerformSpecialAttack()) return;

	LastSpecialAttackTime = GetWorld()->GetTimeSeconds();

	ApplyAreaDamage(SpecialAttackDamage, SpecialAttackRadius);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
			FString::Printf(TEXT("Base Special Attack! Damage: %.0f, Radius: %.0f"), SpecialAttackDamage, SpecialAttackRadius));
	}
}


// --- Utilidade
float AIABase::GetDistanceToTarget() const
{
	AActor* Player = GetPlayerTarget();
	if (!Player) return -1.0f;
	return FVector::Dist(GetActorLocation(), Player->GetActorLocation());
}

AActor* AIABase::GetPlayerTarget() const
{
	if (const AAIControllerBase* C = Cast<AAIControllerBase>(GetController()))
	{
		if (AActor* T = C->GetTargetActor())
			return T;
	}
	return nullptr;
}

bool AIABase::IsPlayerInRange(float Range) const
{
	const float D = GetDistanceToTarget();
	return (D >= 0.f) && (D <= Range);
}

void AIABase::SetMovementSpeed(float NewSpeed)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
	}
}


void AIABase::HandleDeath()
{
	SetMovementSpeed(0.0f);

	if (AAIControllerBase* C = Cast<AAIControllerBase>(GetController()))
	{
		C->StopMovement();
		C->SetCurrentState(EAIState::Dead);
	}

	Destroy();
}


void AIABase::HandleHealing(float DeltaTime)
{
	if (CurrentHealth < MaxHealth)
	{
		Heal(HealingRate * DeltaTime);
	}
}

void AIABase::ApplyDamageToTarget(AActor* Target, float Damage)
{
	if (!Target) return;

	FPointDamageEvent E;
	E.DamageTypeClass = UDamageType::StaticClass();
	E.Damage = Damage;

	Target->TakeDamage(Damage, E, GetController(), this);
}

void AIABase::ApplyAreaDamage(float Damage, float Radius)
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
				}
			}
		}
	}

	// Debug visual da área de efeito
	if (GEngine)
	{
		DrawDebugSphere(GetWorld(), Origin, Radius, 32, FColor::Red, false, 2.0f, 0, 3.0f);
	}
}