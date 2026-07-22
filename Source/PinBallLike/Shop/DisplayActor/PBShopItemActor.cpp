// Fill out your copyright notice in the Description page of Project Settings.
#include "PBShopItemActor.h"

#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Texture2D.h"

// Sets default values
APBShopItemActor::APBShopItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	ClickCollision =
		CreateDefaultSubobject<UBoxComponent>(
			TEXT("ClickCollision"));

	SetRootComponent(ClickCollision);

	ClickCollision->SetBoxExtent(
		FVector(100.f, 100.f, 100.f));

	ClickCollision->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly);

	ClickCollision->SetCollisionResponseToAllChannels(
		ECR_Ignore);

	ClickCollision->SetCollisionResponseToChannel(
		ECC_Visibility,
		ECR_Block);

	ClickCollision->OnBeginCursorOver.AddDynamic(
		this,
		&APBShopItemActor::HandleBeginCursorOver);

	ClickCollision->OnEndCursorOver.AddDynamic(
		this,
		&APBShopItemActor::HandleEndCursorOver);

	ClickCollision->OnClicked.AddDynamic(
		this,
		&APBShopItemActor::HandleClicked);

	Billboard =
	CreateDefaultSubobject<UBillboardComponent>(
		TEXT("Billboard"));

	Billboard->SetupAttachment(ClickCollision);

	Billboard->SetRelativeLocation(
		FVector(0.f, 0.f, 60.f));

	Billboard->SetUsingAbsoluteScale(true);
	Billboard->SetWorldScale3D(FVector(25.f));

	Billboard->SetVisibility(true);
	Billboard->SetHiddenInGame(false);
}

void APBShopItemActor::SetSprite(UTexture2D* InSprite)
{
	if (!Billboard || !InSprite)
	{
		return;
	}

	Billboard->SetSprite(InSprite);
	
	
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
	UE_LOG(LogTemp, Warning, TEXT("Clicked"));
	if (PurchaseHandler)
	{
		PurchaseHandler->RequestPurchase(SlotIndex);
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


void APBShopItemActor::OnPurchase()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}
FVector APBShopItemActor::GetUIWorldLocation() const
{
	FVector Origin;
	//FVector BoxExtent;

	//GetActorBounds(false, Origin, BoxExtent);

	return Origin + FVector(0.f, 0.f,  30.f);
}

