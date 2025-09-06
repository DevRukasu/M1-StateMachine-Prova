#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portao.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UNavModifierComponent;

UCLASS()
class M1IA_API APortao : public AActor
{
	GENERATED_BODY()

public:
	APortao();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	//Função de abrir o portao
	UFUNCTION(BlueprintCallable, Category = "Gate")
	void OpenGate();

protected:
	//Seletor de Mesh do portao
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	UStaticMeshComponent* GateMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate|NavMesh")
	UBoxComponent* NavBoundsProxy = nullptr;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate|NavMesh")
	UNavModifierComponent* NavModifier = nullptr;

	//Quanto que abre (altura no z)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gate|Movement")
	float OpenHeight = 1000.f;

	//velocidade de abertura, 100 ficou lentao, deixa no 200
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gate|Movement")
	float MoveSpeed = 200.f;

	//Vai atualizar a navmesh para  quando o portao abrir o npc passar
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gate|NavMesh", meta = (ClampMin = "1.0"))
	FVector NavBoundsExtent = FVector(150.f, 200.f, 200.f);

private:
	// Estados do portao (n é statemachine mas serve para eledkwakdakdwka)
	FVector InitialLocation;
	FVector TargetLocation;
	bool bIsOpening = false;
	bool bIsClosing = false;

private:
	//funcao que atualiza a navmesh
	void MarkNavigationDirty() const;

	/** Aplica a área correta no NavModifier (Null = bloqueia, Default = libera) */
	void ApplyClosedArea(bool bClosed) const;
};
