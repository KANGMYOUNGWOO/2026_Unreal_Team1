// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PBSynergyDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBSynergyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffect|Data")
	FName RowName = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusEffect|Visual", meta = (AssetBundles = "UI"))
	TSoftObjectPtr<UTexture2D> Icon;
};
