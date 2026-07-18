// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "UObject/PrimaryAssetId.h"
#include "PBBumperDataAsset.generated.h"

class UTexture2D;
class UNiagaraSystem;

UCLASS(BlueprintType)
class PINBALLLIKE_API UPBBumperDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category = "Bumper|Data")
	bool SetRowNameForImport(FName InRowName);
#endif

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Data")
	FName RowName = NAME_None;
	
	/** UI 표시용 아이콘 */

	// AssetBundles 값은 PBAssetBundleNames와 맞춰야 한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Visual", meta = (AssetBundles = "UI"))
	TSoftObjectPtr<UTexture2D> Icon;

	// AssetBundles 값은 PBAssetBundleNames와 맞춰야 한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Trigger", meta = (AssetBundles = "Gameplay"))
	TSoftClassPtr<APBBumperTriggerActorBase> TriggerClass;

	// AssetBundles 값은 PBAssetBundleNames와 맞춰야 한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Effect", meta = (AssetBundles = "Gameplay"))
	TSoftClassPtr<UPBBumperEffectBase> EffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Effect", meta = (AssetBundles = "Gameplay"))
	TSoftObjectPtr<UNiagaraSystem> ActivationVfx;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Effect", meta = (AssetBundles = "Gameplay"))
	TSoftObjectPtr<UNiagaraSystem> DeliveryVfx;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Effect", meta = (AssetBundles = "Gameplay"))
	TSoftObjectPtr<UNiagaraSystem> ImpactVfx;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Effect", meta = (AssetBundles = "Gameplay"))
	TSoftObjectPtr<UNiagaraSystem> StatusVfx;
};
