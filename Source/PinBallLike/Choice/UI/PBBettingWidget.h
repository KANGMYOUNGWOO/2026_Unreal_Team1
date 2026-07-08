// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBBettingWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;

/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBBettingWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	virtual void NativeConstruct() override;
	
private:
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ExitButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> BetText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NationText1;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NationText2;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Flag1;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Flag2;
	
	
	UFUNCTION()
	void OnExitButtonClicked();
};
