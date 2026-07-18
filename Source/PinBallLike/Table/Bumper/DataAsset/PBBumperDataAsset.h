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

/** 범퍼 RowName을 PrimaryAssetId로 삼고 UI 및 전투용 soft reference를 번들별로 보관합니다. */
UCLASS(BlueprintType)
class PINBALLLIKE_API UPBBumperDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
	/** 시트 파서나 검증 도구가 Primary Asset 논리 ID를 안전하게 기록할 때만 사용한다. */
	UFUNCTION(BlueprintCallable, Category = "Bumper|Data")
	bool SetRowNameForImport(FName InRowName);
#endif

	/** 시트 RowName과 동일한 논리 ID. PrimaryAssetId의 Name으로 사용함 */
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

	/** Effect 시트의 ActivationVfxId를 파서가 실제 에셋 참조로 변환한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Effect", meta = (AssetBundles = "Gameplay"))
	TSoftObjectPtr<UNiagaraSystem> ActivationVfx;

	/** 투사체나 직접 전달 단계에 사용하는 연출. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Effect", meta = (AssetBundles = "Gameplay"))
	TSoftObjectPtr<UNiagaraSystem> DeliveryVfx;

	/** 효과가 실제 적용된 위치에서 한 번 재생하는 연출. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Effect", meta = (AssetBundles = "Gameplay"))
	TSoftObjectPtr<UNiagaraSystem> ImpactVfx;

	/** 실제 효과 수명에 맞춰 대상에 부착하는 지속 연출. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Effect", meta = (AssetBundles = "Gameplay"))
	TSoftObjectPtr<UNiagaraSystem> StatusVfx;
};
