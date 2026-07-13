#include "PBShellCupActor.h"
#include "Components/StaticMeshComponent.h"
#include "PBShellGameActor.h"

APBShellCupActor::APBShellCupActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	CupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CupMesh"));
	CupMesh->SetupAttachment(Root);

	CupMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CupMesh->SetGenerateOverlapEvents(false);

	CupMesh->OnClicked.AddDynamic(
		this,
		&APBShellCupActor::HandleClicked);
}

void APBShellCupActor::BeginPlay()
{
	Super::BeginPlay();

	InitialLocation = GetActorLocation();
}

void APBShellCupActor::SetCupIndex(int32 InIndex)
{
	CupIndex = InIndex;
}

void APBShellCupActor::SetOwnerGame(APBShellGameActor* InGame)
{
	OwnerGame = InGame;
}

void APBShellCupActor::RaiseCup(float Height)
{
	SetActorLocation(
		InitialLocation + FVector(0.f,0.f,Height));
}

void APBShellCupActor::LowerCup()
{
	SetActorLocation(InitialLocation);
}

void APBShellCupActor::HandleClicked(
	UPrimitiveComponent* TouchedComponent,
	FKey ButtonPressed)
{
	if (!OwnerGame)
	{
		return;
	}

	OwnerGame->HandleCupSelected(CupIndex);
}

void APBShellCupActor::ShowPrize()
{
	if (PrizeMesh)
	{
		PrizeMesh->SetHiddenInGame(false);
	}
}

void APBShellCupActor::HidePrize()
{
	if (PrizeMesh)
	{
		PrizeMesh->SetHiddenInGame(true);
	}
}