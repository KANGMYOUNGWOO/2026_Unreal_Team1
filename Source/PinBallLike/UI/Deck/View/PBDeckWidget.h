// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBDeckWidget.generated.h"

class UUniformGridPanel;
class UButton;
/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBDeckWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UButton> Button_Exit;
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UUniformGridPanel> GridPanel_BallList;
	
};
