#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../Interface/IShopPurchaseHandler.h"
#include "PBShopItemActor.generated.h"

class UBillboardComponent;
class UBoxComponent;
class UTexture2D;

UCLASS()
class PINBALLLIKE_API APBShopItemActor : public AActor
{
	GENERATED_BODY()

public:
	APBShopItemActor();

	void SetSprite(UTexture2D* InSprite);
	void SetSlotIndex(int32 InSlotIndex);
	void SetHandler(IIShopPurchaseHandler* InHandler);

	FVector GetUIWorldLocation() const;
	void OnPurchase();

private:
	UFUNCTION()
	void HandleBeginCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void HandleEndCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void HandleClicked(
		UPrimitiveComponent* TouchedComponent,
		FKey ButtonPressed);

	void SetHovered(bool bIsHovered);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> ClickCollision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBillboardComponent> Billboard;

	int32 SlotIndex = INDEX_NONE;

	IIShopPurchaseHandler* PurchaseHandler = nullptr;
};