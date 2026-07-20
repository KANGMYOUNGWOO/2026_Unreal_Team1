// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBPurchaseConfirmWidget.generated.h"

class UTextBlock;
class UImage;
class UWidgetAnimation;
class UButton;
/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPurchaseConfirmed,
	int32,
	SlotIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnPurchaseCancelled);

UCLASS()
class PINBALLLIKE_API UPBPurchaseConfirmWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PriceText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SynergyText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> OpenAnim;
	
private:
	
	int32 SlotIndex = INDEX_NONE;
	
	UFUNCTION()
	void OnConfirmButtonClicked();

	UFUNCTION()
	void OnCancelButtonClicked();

	UFUNCTION()
	void OnOpenAnimationFinished();
	
	void SetButtonsEnabled(bool bEnabled);
	
	FWidgetAnimationDynamicEvent OpenAnimationFinishedEvent;

public:
	
	void SetInfo(int32 Index, FText Name, int32 Price, FText Synergy , UTexture2D* Icon);
	
	void PlayOpenAnimation();
	
	UPROPERTY(BlueprintAssignable)
	FOnPurchaseConfirmed OnPurchaseConfirmed;

	UPROPERTY(BlueprintAssignable)
	FOnPurchaseCancelled OnPurchaseCancelled;
};
