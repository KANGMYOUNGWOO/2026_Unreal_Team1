// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopItemActor.h"

// Sets default values
APBShopItemActor::APBShopItemActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	SetRootComponent(Mesh);
	
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	Mesh->SetCollisionResponseToChannel(
		ECC_Visibility,
		ECR_Block);
	
	Mesh->OnBeginCursorOver.AddDynamic(this, &APBShopItemActor::HandleBeginCursorOver);
	
	Mesh->OnEndCursorOver.AddDynamic(this, &APBShopItemActor::HandleEndCursorOver);
	
	Mesh->OnClicked.AddDynamic(this, &APBShopItemActor::HandleClicked);
	
}

void APBShopItemActor::SetMesh(UStaticMesh* InMesh)
{
	if (!Mesh || !InMesh)
	{
		return;
	}
	
	Mesh->SetStaticMesh(InMesh);
	
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void APBShopItemActor::SetSlotIndex(int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;
}

void APBShopItemActor::SetHandler(IIShopPurchaseHandler* handler)
{
	PurchaseHandler = handler;
}

// Called when the game starts or when spawned
void APBShopItemActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void APBShopItemActor::HandleBeginCursorOver(UPrimitiveComponent* TouchedComponent)
{
	SetHovered(true);
	
}

void APBShopItemActor::HandleEndCursorOver(UPrimitiveComponent* TouchedComponent)
{
	SetHovered(false);
}

void APBShopItemActor::HandleClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	if (PurchaseHandler)
	{
		bool IsSuccess = PurchaseHandler->BuyItem(SlotIndex);
		
		if (IsSuccess)
		{
			Destroy();
			
			//SetActorHiddenInGame(!IsSuccess);
			//SetActorEnableCollision(!IsSuccess);
			//SetActorTickEnabled(!IsSuccess);
		}
	}
}

void APBShopItemActor::SetHovered(bool IsHovered)
{
	if (IsHovered)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hover"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Hover End"));
	}
}

// Called every frame
void APBShopItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FVector APBShopItemActor::GetUIWorldLocation() const
{
	FVector Origin;
	FVector BoxExtent;

	GetActorBounds(false, Origin, BoxExtent);

	return Origin + FVector(0.f, 0.f, BoxExtent.Z + 30.f);
}
