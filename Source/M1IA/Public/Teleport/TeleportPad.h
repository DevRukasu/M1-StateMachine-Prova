#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TeleportPad.generated.h"

UCLASS()
class M1IA_API ATeleportPad : public AActor
{
    GENERATED_BODY()

public:
    ATeleportPad();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UBoxComponent* CollisionBox; //criar o componente no objeto

    //Janela para colocar o nome do level, fica mais facil se tiver varios leveis, coisa q tem
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport")
    FName LevelToLoad;

    //Função base da unreal, foi um saco achar essa porra
    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );
};