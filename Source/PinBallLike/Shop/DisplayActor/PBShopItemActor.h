// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../Interface/IShopPurchaseHandler.h"
#include "PBShopItemActor.generated.h"

UCLASS()
class PINBALLLIKE_API APBShopItemActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APBShopItemActor();
	
	void SetMesh(UStaticMesh* InMesh);

	void SetSlotIndex(int32 InSlotIndex);
	
	void SetHandler(IIShopPurchaseHandler* handler);
	
	FVector GetUIWorldLocation() const;
	
	int32 GetSlotIndex() const
	{
		return SlotIndex;
	}
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void HandleBeginCursorOver(UPrimitiveComponent* TouchedComponent);
	
	UFUNCTION()
	void HandleEndCursorOver(UPrimitiveComponent* TouchedComponent);
	
	UFUNCTION()
	void HandleClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	
	
private:
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	void SetHovered(bool IsHovered);
	
	int32 SlotIndex = INDEX_NONE;
	
	IIShopPurchaseHandler* PurchaseHandler;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public :
	
	
	

	
};
