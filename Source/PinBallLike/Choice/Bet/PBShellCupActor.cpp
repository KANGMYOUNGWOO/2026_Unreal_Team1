#include "PBShellCupActor.h"
#include "Components/BillboardComponent.h"
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

	PrizeBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("PrizeBillboard"));
	PrizeBillboard->SetupAttachment(Root);
	
	
	CupMesh->OnClicked.AddDynamic(
		this,
		&APBShellCupActor::HandleClicked);
}

void APBShellCupActor::BeginPlay()
{
	Super::BeginPlay();

	if (CupMesh)
	{
		InitialCupMeshRelativeLocation =
			CupMesh->GetRelativeLocation();
	}

	HidePrize();
}

void APBShellCupActor::SetCupIndex(int32 InIndex)
{
	CupIndex = InIndex;
}

void APBShellCupActor::SetOwnerGame(APBShellGameActor* InGame)
{
	OwnerGame = InGame;
}

void APBShellCupActor::SetSelectionEnabled(const bool bEnabled)
{
	bSelectionEnabled = bEnabled;
}

void APBShellCupActor::RaiseCup(float Height)
{
	bRaisePresentationPending = true;
	bLowerPresentationPending = false;
	PlayRaiseCup(Height);
}

void APBShellCupActor::LowerCup()
{
	bRaisePresentationPending = false;
	bLowerPresentationPending = true;
	PlayLowerCup();
}

void APBShellCupActor::ResetCupPresentation()
{
	bRaisePresentationPending = false;
	bLowerPresentationPending = false;
	PlayResetCupPresentation();
}

void APBShellCupActor::NotifyRaiseCupFinished()
{
	if (!bRaisePresentationPending)
	{
		return;
	}

	bRaisePresentationPending = false;
	OnRaiseCupFinished.Broadcast(this);
}

void APBShellCupActor::NotifyLowerCupFinished()
{
	if (!bLowerPresentationPending)
	{
		return;
	}

	bLowerPresentationPending = false;
	OnLowerCupFinished.Broadcast(this);
}

void APBShellCupActor::PlayRaiseCup_Implementation(float Height)
{
	if (!CupMesh)
	{
		NotifyRaiseCupFinished();
		return;
	}

	CupMesh->SetRelativeLocation(
		InitialCupMeshRelativeLocation +
		FVector(0.f, 0.f, Height));

	NotifyRaiseCupFinished();
}

void APBShellCupActor::PlayLowerCup_Implementation()
{
	if (!CupMesh)
	{
		NotifyLowerCupFinished();
		return;
	}

	CupMesh->SetRelativeLocation(
		InitialCupMeshRelativeLocation);

	NotifyLowerCupFinished();
}

void APBShellCupActor::PlayResetCupPresentation_Implementation()
{
	if (!CupMesh)
	{
		return;
	}

	CupMesh->SetRelativeLocation(
		InitialCupMeshRelativeLocation);
}

void APBShellCupActor::HandleClicked(
	UPrimitiveComponent* TouchedComponent,
	FKey ButtonPressed)
{
	if (!bSelectionEnabled || !OwnerGame)
	{
		return;
	}

	OwnerGame->HandleCupSelected(CupIndex);
}

void APBShellCupActor::ShowPrize()
{
	if (PrizeBillboard)
	{
		PrizeBillboard->SetHiddenInGame(false);
	}
}

void APBShellCupActor::HidePrize()
{
	if (PrizeBillboard)
	{
		PrizeBillboard->SetHiddenInGame(true);
	}
}

void APBShellCupActor::SetPrizeSprite(UTexture2D* PrizeTexture)
{
	if (PrizeBillboard)
	{
		PrizeBillboard->SetSprite(PrizeTexture);
	}
}

void APBShellCupActor::SetPrizeScale(const float UniformScale)
{
	if (PrizeBillboard)
	{
		PrizeBillboard->SetRelativeScale3D(
			FVector(FMath::Max(UniformScale, 0.f)));
	}
}
