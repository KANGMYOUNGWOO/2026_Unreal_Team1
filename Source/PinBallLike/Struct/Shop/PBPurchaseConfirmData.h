// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Synergy/PBSynergyViewData.h"
#include "PBPurchaseConfirmData.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBPurchaseConfirmData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	FText BallName = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly)
	FText Description = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly)
	int32 Price = 0;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTexture2D> BallIcon = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<FPBSynergyViewData> Synergies;

	UPROPERTY(BlueprintReadOnly)
	int32 HP = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 MP = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Attack = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ManaRegen = 0;
};