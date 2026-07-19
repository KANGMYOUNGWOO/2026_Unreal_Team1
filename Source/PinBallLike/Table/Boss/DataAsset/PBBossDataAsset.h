// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossPatternBase.h"
#include "PinBallLike/Actor/Boss/UI/PBBossUILayerWidget.h"
#include "PinBallLike/Table/Boss/Struct/PBBossHitPointData.h"
#include "PinBallLike/Table/Boss/Struct/PBBossPatternData.h"
#include "PBBossDataAsset.generated.h"

class UCameraShakeBase;
class UTexture2D;

UCLASS(BlueprintType)
class PINBALLLIKE_API UPBBossDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Data")
	FName RowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Actor", meta = (AssetBundles = "Gameplay"))
	TSoftClassPtr<APBBossBase> BossClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Profile")
	FText BossName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Profile", meta = (AssetBundles = "UI"))
	TSoftObjectPtr<UTexture2D> BossIntroImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Stat", meta = (ClampMin = "1"))
	int32 MaxHP = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Stat", meta = (ClampMin = "0", ClampMax = "100"))
	int32 EnrageHPRatioPercent = 40;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Groggy", meta = (ClampMin = "1"))
	int32 MaxGroggyGauge = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Groggy", meta = (ClampMin = "0.1"))
	float GroggyDurationSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Damage")
	FName DefaultHitPointName = TEXT("Normal");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Damage", meta = (ClampMin = "0"))
	float DamageCooldownSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|HitPoint")
	TArray<FPBBossHitPointData> HitPointDatas;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Pattern", meta = (ClampMin = "0"))
	float MinPatternIntervalSeconds = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Pattern", meta = (ClampMin = "0.1"))
	float PatternCheckIntervalSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Pattern", meta = (AssetBundles = "Gameplay"))
	TArray<FPBBossPatternData> PatternDatas;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Pattern", meta = (AssetBundles = "Gameplay"))
	TArray<FPBBossPatternData> EnragedPatternDatas;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Pattern", meta = (AssetBundles = "Gameplay"))
	TArray<FPBBossPatternData> EnragedEntryPatternDatas;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|UI", meta = (AssetBundles = "UI"))
	TSoftClassPtr<UPBBossUILayerWidget> BossUILayerClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|UI", meta = (AssetBundles = "Gameplay"))
	TSoftClassPtr<UCameraShakeBase> EnrageCameraShakeClass;
};
