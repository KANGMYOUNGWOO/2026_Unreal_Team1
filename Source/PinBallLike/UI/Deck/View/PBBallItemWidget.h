// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBBallItemWidget.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBBallItemWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UImage> Image_ProgressBar;
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UImage> Image_Ball;
};
