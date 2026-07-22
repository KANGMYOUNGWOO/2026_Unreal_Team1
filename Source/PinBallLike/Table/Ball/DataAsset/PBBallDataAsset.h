// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
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
	TSoftObjectPtr<UTexture2D> BallIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Visual", meta = (AssetBundles = "Gameplay,UI"))
	TSoftObjectPtr<UTexture2D> BallSprite;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Visual", meta = (AssetBundles = "UI"))
	TSoftObjectPtr<UTexture2D> PowerFlipIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Visual", meta = (AssetBundles = "UI"))
	TSoftObjectPtr<UTexture2D> ClassIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Visual", meta = (AssetBundles = "UI"))
	TArray<TSoftObjectPtr<UTexture2D>> RaceIcons;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Skill", meta = (AssetBundles = "Gameplay"))
	TSoftClassPtr<APBBallSkillActorBase> SkillActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Skill", meta = (AssetBundles = "UI"))
	TSoftObjectPtr<UTexture2D> SkillIcon;
};
