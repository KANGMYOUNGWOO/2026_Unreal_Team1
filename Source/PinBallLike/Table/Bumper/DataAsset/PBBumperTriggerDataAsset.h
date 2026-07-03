// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/PrimaryAssetId.h"
#include "PBBumperTriggerDataAsset.generated.h"

class APBBumperTriggerActorBase;

UCLASS(BlueprintType)
class PINBALLLIKE_API UPBBumperTriggerDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** 시트 RowName과 동일한 논리 ID. PrimaryAssetId의 Name으로 사용함 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Data")
	FName RowName = NAME_None;

	/** 트리거를 실제로 생성할 때 사용하는 액터 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger", meta = (AssetBundles = "Gameplay"))
	TSoftClassPtr<APBBumperTriggerActorBase> TriggerClass;
};
