#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IA/EnumStateMachine.h"
#include "Engine/DamageEvents.h"
#include "IABase.generated.h"

UCLASS()
class M1IA_API AIABase : public ACharacter
{
	GENERATED_BODY()

public:
	AIABase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public: 
	//Vida
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float HealingRate = 10.0f;




	//Movimento
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ChaseSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float PatrolSpeed = 200.0f;




	//Combate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MeleeAttackDamage = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float RangedAttackDamage = 10.0f;

	// Valores de combate que ambos compartilham
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MeleeAttackRange = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float RangedAttackRange = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 1.5f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float LastAttackTime = 0.0f;




	//Special
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Special")
	float SpecialAttackDamage = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Special")
	float SpecialAttackRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Special")
	float SpecialAttackCooldown = 5.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Special")
	float LastSpecialAttackTime = 0.0f;

	//Detecção das ia
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	float ChaseDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	float AlertDistance = 2000.0f;

	// Retreat
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Retreat")
	FVector RetreatLocation = FVector(0.0f, 0.0f, 0.0f);

public:

	//Funçao de combate
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsDead() const { return CurrentHealth <= 0.0f; }

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealthPercentage() const { return CurrentHealth / MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanAttack() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void PerformMeleeAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void PerformRangedAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void PerformSpecialAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanPerformSpecialAttack() const;



	// Utilidade
	UFUNCTION(BlueprintCallable, Category = "AI Utility")
	float GetDistanceToTarget() const;

	UFUNCTION(BlueprintCallable, Category = "AI Utility")
	AActor* GetPlayerTarget() const;

	UFUNCTION(BlueprintCallable, Category = "AI Utility")
	bool IsPlayerInRange(float Range) const;

	UFUNCTION(BlueprintCallable, Category = "AI Utility")
	void SetMovementSpeed(float NewSpeed);



	// Getters necessários para o Controller
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetMeleeAttackRange() const { return MeleeAttackRange; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetRangedAttackRange() const { return RangedAttackRange; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetAttackCooldown() const { return AttackCooldown; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual float GetSpecialAttackCooldown() const { return SpecialAttackCooldown; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual float GetSpecialAttackRadius() const { return SpecialAttackRadius; }

	UFUNCTION(BlueprintCallable, Category = "Detection")
	float GetChaseDistance() const { return ChaseDistance; }

protected:

	// Interno
	virtual void HandleDeath();
	virtual void HandleHealing(float DeltaTime);

	void ApplyDamageToTarget(AActor* Target, float Damage);
	virtual void ApplyAreaDamage(float Damage, float Radius);
};
