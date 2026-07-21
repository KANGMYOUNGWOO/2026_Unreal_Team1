// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "PBPurchaseConfirmData.generated.h"


USTRUCT(BlueprintType)
struct FPBPurchaseConfirmData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	FName BallId = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	FText BallName;

	UPROPERTY(BlueprintReadOnly)
	FText BallDescription;

	UPROPERTY(BlueprintReadOnly)
	int32 Price = 0;

	UPROPERTY(BlueprintReadOnly)
	FText SynergyName;

	UPROPERTY(BlueprintReadOnly)
	FText SynergyDescription;

	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> BallIcon;
};