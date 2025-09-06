#include "Teleport/TeleportPad.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Character.h"

ATeleportPad::ATeleportPad()
{
	PrimaryActorTick.bCanEverTick = false;

	//Cria a collision para o player pisar
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;

	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	//Para quem responde?
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ATeleportPad::OnOverlapBegin);
}

//função base dificil da unreal
void ATeleportPad::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	//so teleporta se for um ACharacter, no caso o jogador
	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		if (!LevelToLoad.IsNone())
		{
			UGameplayStatics::OpenLevel(this, LevelToLoad);
		}
	}
}
