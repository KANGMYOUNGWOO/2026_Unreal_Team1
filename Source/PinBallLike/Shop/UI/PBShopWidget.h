// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBShopWidget.generated.h"

/**
 * 
 */
class UCanvasPanel;
class UPBShopSlotWidget;
class UTextBlock; 
struct FBallDataStruct;

UCLASS()
class PINBALLLIKE_API UPBShopWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
	
public :
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPBShopSlotWidget> Slot0;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPBShopSlotWidget> Slot1;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPBShopSlotWidget> Slot2;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPBShopSlotWidget> Slot3;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPBShopSlotWidget> Slot4;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPBShopSlotWidget> Slot5;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPBShopSlotWidget> Slot6;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPBShopSlotWidget> Slot7;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> GoldText;
	
	
private:
	void UpdateSlotWidgetPositionsOnce();
	
public:
	virtual void NativeConstruct() override;

	void SetShopSlotWorldLocations(const TArray<FVector>& InWorldLocations);
	
	void SetShopSlotWidgetData(TArray<const FBallDataStruct*> BallDatas);
	
	void SetShopSlotWidgetData(int32 index, FText Name, int32 Price, FText Synergy);
	
	void UnActiveSlotWidget(int32 SlotIndex);

private:
	

	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> ShopCanvas;
	
	TArray<TObjectPtr<UPBShopSlotWidget>> ShopSlotWidgets;

	UPROPERTY()
	TArray<FVector> ShopSlotWorldLocations;

	UPROPERTY(EditAnywhere, Category = "Shop UI")
	FVector SlotWidgetWorldOffset = FVector(0.f, -30.f, 40.f);
};
