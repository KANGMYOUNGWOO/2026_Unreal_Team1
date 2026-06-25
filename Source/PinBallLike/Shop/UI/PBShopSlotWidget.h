// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBShopSlotWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBShopSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	void SetSlotInfo(const FText& NameText, int32 Price , const FText& SynergyText);
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameTextBlock;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PriceTextBlock;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SynergyTextBlock;
};
