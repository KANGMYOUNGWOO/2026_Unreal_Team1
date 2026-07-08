// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBChoiceWidget.generated.h"

class UButton;
class APBChoiceRouteActor;
/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBChoiceWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
public :
	
	void SetChoiceRouteActor(APBChoiceRouteActor* choiceRouteActor);
	
protected:
	
	void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LeftButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RightButton;

	UFUNCTION()
	void HandleLeftButtonClicked();

	UFUNCTION()
	void HandleRightButtonClicked();
	
	APBChoiceRouteActor* ChoiceRouteActor;
	
	
private :
	bool IsSelected = false;
};
