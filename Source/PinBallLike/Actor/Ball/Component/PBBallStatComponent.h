// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinBallLike/Interface/BallStat.h"
#include "PBBallStatComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBallStatComponent();

	int32 GetStat(EBallStatType InType) const;
	void SetStat(EBallStatType InType, int32 Value);
	void ApplyStat(EBallStatType InType, int32 Delta);
	

protected:
	virtual void BeginPlay() override;
	
private:
	int32 StatTypeToIndex(EBallStatType InType) const;
	TArray<int32> Stats;
	
};
