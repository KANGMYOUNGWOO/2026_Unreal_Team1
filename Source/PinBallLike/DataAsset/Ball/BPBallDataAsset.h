// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Struct/Ball/PBBallStarLevelData.h"
#include "PinBallLike/Struct/Ball/PBPowerFlipType.h"
#include "BPBallDataAsset.generated.h"

UCLASS(BlueprintType)
class UPBBallDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 1. 기본 정보
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Base Info")
	int32 BallId;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Base Info")
	EPBPowerFlipType PowerFlipType;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Base Info")
	TArray<int32> SynergyIds;
	
	// 2. 시각 및 에셋 정보
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
	FName NameKey;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
	FName DescriptionKey;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
	TSubclassOf<APBBallBase> Ball;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
	TObjectPtr<class UTexture2D> Icon;
	
	// 3. 인게임 성장 데이터 (성급)
	// Key: 성급 레벨 (1, 2, 3) / Value: 해당 성급의 스탯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progression|InGame")
	TMap<int32, FPBBallStarLevelData> StarLevelData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill")
	TArray<int32> SkillIds;
};