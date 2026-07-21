// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBShopHoverInfoWidget.generated.h"

class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBShopHoverInfoWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
public:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PriceText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SynergyText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;
};
