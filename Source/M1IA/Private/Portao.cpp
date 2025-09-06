#include "Portao.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Null.h"
#include "NavigationSystem.h"           
#include "Kismet/KismetMathLibrary.h"

APortao::APortao()
{
	PrimaryActorTick.bCanEverTick = true;

	//Cria componente de mesh para escolher que tipo de objeto aparece
	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	RootComponent = GateMesh;

	// Caixa proxy para “desenhar” a área da passagem na NavMesh
	NavBoundsProxy = CreateDefaultSubobject<UBoxComponent>(TEXT("NavBoundsProxy"));
	NavBoundsProxy->SetupAttachment(RootComponent);
	NavBoundsProxy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NavBoundsProxy->SetGenerateOverlapEvents(false);
	NavBoundsProxy->SetHiddenInGame(true);
	NavBoundsProxy->SetBoxExtent(NavBoundsExtent, false);

	// Componente que altera a área de navegação nos bounds do ator
	NavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifier"));
}

void APortao::BeginPlay()
{
	Super::BeginPlay();

	//Garante tamanho dessa caixa
	if (NavBoundsProxy)
	{
		NavBoundsProxy->SetBoxExtent(NavBoundsExtent, false);
	}

	InitialLocation = GetActorLocation();
	TargetLocation = InitialLocation + FVector(0.f, 0.f, OpenHeight);

	
	ApplyClosedArea(true);
	MarkNavigationDirty();
}

void APortao::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsOpening)
	{
		const FVector NewLocation = FMath::VInterpConstantTo(GetActorLocation(), TargetLocation, DeltaSeconds, MoveSpeed);
		SetActorLocation(NewLocation);

		// quando abre o portao a navmesh atualiza permitindo o NPC de atravessar
		ApplyClosedArea(false);
		MarkNavigationDirty();

		if (FVector::DistSquared(NewLocation, TargetLocation) <= 1.f)
		{
			bIsOpening = false;
			
			ApplyClosedArea(false);
			MarkNavigationDirty();
		}
	}
	else if (bIsClosing)
	{
		const FVector NewLocation = FMath::VInterpConstantTo(GetActorLocation(), InitialLocation, DeltaSeconds, MoveSpeed);
		SetActorLocation(NewLocation);

		// Enquanto fecha, já bloqueia
		ApplyClosedArea(true);
		MarkNavigationDirty();

		if (FVector::DistSquared(NewLocation, InitialLocation) <= 1.f)
		{
			bIsClosing = false;
			ApplyClosedArea(true);
			MarkNavigationDirty();
		}
	}
}

//Abrir portao
void APortao::OpenGate()
{
	bIsClosing = false;
	bIsOpening = true;

	InitialLocation = GetActorLocation();     // posicao inicial é a que ele ta no jogo, simples           
	TargetLocation = InitialLocation + FVector(0.f, 0.f, OpenHeight);

	ApplyClosedArea(false);
	MarkNavigationDirty();
}

//controla se a area pode ser andavel pela ia ou nao
void APortao::ApplyClosedArea(bool bClosed) const
{
	if (!NavModifier) return;

	NavModifier->SetAreaClass(bClosed ? UNavArea_Null::StaticClass() : nullptr);


	NavModifier->SetNavigationRelevancy(true);
	if (GateMesh)
	{
		GateMesh->SetCanEverAffectNavigation(true);
	}
}


//Recalcula a area para o npc andar 
void APortao::MarkNavigationDirty() const
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World))
	{
		// Usa a caixa proxy se existir, senão usa o bounds do mesh
		FBox DirtyBox(ForceInit);
		if (NavBoundsProxy)
		{
			DirtyBox = NavBoundsProxy->Bounds.GetBox();
		}
		else if (GateMesh)
		{
			DirtyBox = GateMesh->Bounds.GetBox();
		}
		else
		{
			DirtyBox = FBox(GetActorLocation() - FVector(200.f), GetActorLocation() + FVector(200.f));
		}

		NavSys->AddDirtyArea(DirtyBox, ENavigationDirtyFlag::All);
	}
}
