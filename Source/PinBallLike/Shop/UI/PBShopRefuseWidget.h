// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBShopRefuseWidget.generated.h"

class UWidgetAnimation;

/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBShopRefuseWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void Show();
	
protected:
	UFUNCTION()
	void OnRefuseAnimationFinished();

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> RefuseAnim;
	
	FWidgetAnimationDynamicEvent RefuseAnimationFinishedEvent;
	
};
