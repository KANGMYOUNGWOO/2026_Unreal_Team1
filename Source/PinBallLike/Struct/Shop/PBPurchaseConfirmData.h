// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Ball/PBBallDetailTooltipViewData.h"
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
	int32 Price = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bHasBall = false;

	UPROPERTY(BlueprintReadOnly)
	FName BallId = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTexture2D> BallIconTexture = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FText BallNameText;

	UPROPERTY(BlueprintReadOnly)
	FText BallDescriptionText;

	UPROPERTY(BlueprintReadOnly)
	FPBBallDetailInfoRowViewData HpRow;

	UPROPERTY(BlueprintReadOnly)
	FPBBallDetailInfoRowViewData MpRow;

	UPROPERTY(BlueprintReadOnly)
	FPBBallDetailInfoRowViewData AttackRow;

	UPROPERTY(BlueprintReadOnly)
	FPBBallDetailInfoRowViewData ManaRegenRow;

	UPROPERTY(BlueprintReadOnly)
	FPBBallDetailIconTextViewData PowerFlipData;

	UPROPERTY(BlueprintReadOnly)
	FPBBallDetailIconTextViewData ClassData;

	UPROPERTY(BlueprintReadOnly)
	TArray<FPBBallDetailIconTextViewData> RaceDataList;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTexture2D> SkillIconTexture = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FText SkillNameText;

	UPROPERTY(BlueprintReadOnly)
	FText SkillDescriptionText;
};