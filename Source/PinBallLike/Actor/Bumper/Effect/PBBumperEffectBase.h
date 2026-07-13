// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PBBumperEffectBase.generated.h"

class APBBallBase;
class APBModularBumperBase;

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

	UFUNCTION(BlueprintCallable, Category = "Bumper|Effect")
	virtual void FinishEffect();

	UFUNCTION(BlueprintPure, Category = "Bumper|Effect")
	FPBBumperEffectRow GetEffectData() const;

protected:
	UPROPERTY()
	TWeakObjectPtr<APBModularBumperBase> OwnerBumper;

	/** Effect 시트의 현재 행. Power 등 효과별 수치는 이 값에서 읽는다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Effect")
	FPBBumperEffectRow EffectData;
};
