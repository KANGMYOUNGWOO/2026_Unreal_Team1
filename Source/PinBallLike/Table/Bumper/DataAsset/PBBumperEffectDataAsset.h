// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/PrimaryAssetId.h"
#include "PBBumperEffectDataAsset.generated.h"

class UPBBumperEffectBase;

UCLASS(BlueprintType)
class PINBALLLIKE_API UPBBumperEffectDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** 시트 RowName과 동일한 논리 ID. PrimaryAssetId의 Name으로 사용함 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Data")
	FName RowName = NAME_None;

	/** 범퍼 발동 시 실행할 효과 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Effect", meta = (AssetBundles = "Gameplay"))
	TSoftClassPtr<UPBBumperEffectBase> EffectClass;
};
