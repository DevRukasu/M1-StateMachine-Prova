#pragma once

#include "CoreMinimal.h"
#include "IA/IABase.h"
#include "IAGuardiao.generated.h"

class APortao;

UCLASS()
class M1IA_API AIAGuardiao : public AIABase
{
	GENERATED_BODY()

public:
	AIAGuardiao();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	//Alert
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert")
	float AlertMinTime = 2.0f;

	//So serve para definir segundos no debug
	float AlertTimer = 0;



	//Special
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Guardian Special")
	float SpecialAttackHealthThreshold = 0.4f; // 40% da vida mas da para mudar no editor para testes



	//Retreat
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Retreat")
	float RetreatHealthThreshold = 0.2f; // 20% da vida da IA Guardia

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Retreat")
	TObjectPtr<APortao> GateToOpen; //Ponteiro para o portao que ta no mapa, ja que ele controla



	//Bools de controles
	UPROPERTY(BlueprintReadOnly, Category = "AI|Guardian")
	bool bHasCalledReinforcements = false;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Guardian")
	bool bHasRetreatStarted = false;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Guardian")
	bool bGateOpened = false;

public:

	//Abaixo tem funções de Notificação, nao usei delegates por ser mais complexo e n ter tempo entao vai assim
	virtual void PerformSpecialAttack() override;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanPerformGuardianSpecialAttack() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool ShouldActivateSpecialAttack() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool ShouldStartRetreat() const;

	UFUNCTION(BlueprintCallable, Category = "AI")
	bool IsPlayerInAlertRange() const;




	//Falto essa porra para n bugar
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Retreat")
	bool bHasFinishedRetreat = false;

	UFUNCTION(BlueprintCallable, Category = "Retreat")
	void StartRetreat();




	//Função que abre o portao baseado na IA
	UFUNCTION(BlueprintCallable, Category = "Retreat")
	void OpenGate();


	//como lida com Alerta e Retreat
	void HandleAlertBehavior();

	void HandleRetreatMovement();
};
