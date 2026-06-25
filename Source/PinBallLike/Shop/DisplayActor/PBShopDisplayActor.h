// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Subsystem/BallDataStruct.h"
#include "PBShopDisplayActor.generated.h"

class APBShopItemActor;

UCLASS()
class PINBALLLIKE_API APBShopDisplayActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APBShopDisplayActor();

	void DisplayItems(TArray<int32> ItemsID);
	
	TArray<FVector> GetSlotWorldLocation() const;
	
	void ClearItems();
	
private:
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;
	
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<USceneComponent>> ShopSlots;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<APBShopItemActor> ShopItemActorClass;
	
	UPROPERTY()
	TArray<TObjectPtr<APBShopItemActor>> SpawnedItems;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
