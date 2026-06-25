// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinBallLike/Interface/StatProvider.h"
#include "PBBaseStatComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FPBOnStatChanged, FName, int32);

UCLASS(ClassGroup=(PinBall), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBaseStatComponent : public UActorComponent, public IStatProvider
{
	GENERATED_BODY()

public:
	UPBBaseStatComponent();

	FPBOnStatChanged OnStatChanged;

	UFUNCTION(BlueprintCallable, Category="Stat")
	virtual bool HasStat(FName StatName) const override;

	UFUNCTION(BlueprintCallable, Category="Stat")
	virtual int32 GetStat(FName StatName) const override;

	UFUNCTION(BlueprintCallable, Category="Stat")
	void SetStat(FName StatName, int32 Value);

	UFUNCTION(BlueprintCallable, Category="Stat")
	void ApplyStat(FName StatName, int32 Delta);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stat", meta=(AllowPrivateAccess="true"))
	TMap<FName, int32> Stats;
};
