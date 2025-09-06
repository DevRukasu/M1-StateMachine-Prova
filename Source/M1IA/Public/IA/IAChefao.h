#pragma once

#include "CoreMinimal.h"
#include "IA/IABase.h"
#include "IAChefao.generated.h"

UCLASS()
class M1IA_API AIAChefao : public AIABase
{
	GENERATED_BODY()

public:
	AIAChefao();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// === Combate especial do chefe ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Boss Special")
	float BossSpecialAttackDamage = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Boss Special")
	float BossSpecialAttackRadius = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Boss Special")
	float BossSpecialAttackCooldown = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Boss Special")
	float SpecialAttackHealthThreshold = 0.5f; // 50% da vida

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Boss Special")
	float LastBossSpecialAttackTime = 0.0f;

	// Cura do chefe 
	UPROPERTY(BlueprintReadOnly, Category = "AI|Healing")
	bool bHasHealedOnce = false;

	// Controle de uso do Special Attack
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Boss Special")
	bool bCanUseSpecialAttack = false;

public:
	// === Métodos ===
	virtual void PerformSpecialAttack() override;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanPerformBossSpecialAttack() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool ShouldActivateSpecialAttack() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsPlayerInSpecialAttackRange() const;

	// Override dos getters para usar as propriedades do Boss
	virtual float GetSpecialAttackCooldown() const override { return BossSpecialAttackCooldown; }
	virtual float GetSpecialAttackRadius() const override { return BossSpecialAttackRadius; }

	// Healing
	void HandleHealing(float DeltaTime);
	bool IsRecovering() const;

protected:
	virtual void ApplyAreaDamage(float Damage, float Radius) override;
};
