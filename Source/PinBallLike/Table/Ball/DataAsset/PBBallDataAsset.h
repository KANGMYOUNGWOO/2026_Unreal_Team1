// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PBBallDataAsset.generated.h"

class UTexture2D;
class APBBallSkillActorBase;

UCLASS()
class PINBALLLIKE_API UPBBallDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Data")
	FName RowName = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Visual", meta = (AssetBundles = "UI"))
	TSoftObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Actor", meta = (AssetBundles = "Gameplay"))
	TSoftClassPtr<APBBallBase> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Skill", meta = (AssetBundles = "Gameplay"))
	TSoftClassPtr<APBBallSkillActorBase> SkillActorClass;
};
