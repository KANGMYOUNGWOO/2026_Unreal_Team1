// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PBBumperEffectBase.generated.h"

class APBBallBase;
class APBModularBumperBase;
class AActor;

/** 시트 Effect Row와 소유 범퍼를 받아 모든 범퍼 효과의 공통 실행 및 VFX API를 제공합니다. */
UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API UPBBumperEffectBase : public UObject
{
	GENERATED_BODY()

public:
	/** 공통 데이터 주입 후 하위 클래스의 기존 Initialize 흐름을 호출한다. */
	void InitializeEffect(
		APBModularBumperBase* InOwnerBumper,
		const FPBBumperEffectRow& InEffectData);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Effect")
	virtual void Initialize(APBModularBumperBase* InOwnerBumper);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Effect")
	virtual void ActivateEffect(APBModularBumperBase* Bumper, APBBallBase* Ball);

	/** APBBallBase 상속 여부와 무관하게 효과 대상 Actor를 전달하는 공통 실행 경로다. */
	UFUNCTION(BlueprintCallable, Category = "Bumper|Effect")
	virtual void ActivateEffectForActor(APBModularBumperBase* Bumper, AActor* InteractionActor);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Effect")
	virtual void FinishEffect();

	/** Owner 범퍼가 제거될 때 남은 비동기 작업과 외부 액터 참조를 정리합니다. */
	virtual void ShutdownEffect();

	UFUNCTION(BlueprintPure, Category = "Bumper|Effect")
	FPBBumperEffectRow GetEffectData() const;

protected:
	/** 직접 적용 효과의 짧은 전달 연출을 대상에 부착합니다. 투사체 효과는 투사체가 직접 소유합니다. */
	void PlayDeliveryVfx(AActor* TargetActor, float Duration = 0.55f) const;
	void PlayImpactVfx(AActor* TargetActor) const;
	void PlayStatusVfx(AActor* TargetActor, float Duration) const;
	void StopStatusVfx(AActor* TargetActor) const;
	void PlayResolvedVfx(AActor* TargetActor, float StatusDuration = 0.0f, bool bPlayStatus = false) const;
	FName MakeVfxChannel(const TCHAR* Stage) const;

	UPROPERTY()
	TWeakObjectPtr<APBModularBumperBase> OwnerBumper;

	/** Effect 시트의 현재 행. Power 등 효과별 수치는 이 값에서 읽는다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Effect")
	FPBBumperEffectRow EffectData;
};
