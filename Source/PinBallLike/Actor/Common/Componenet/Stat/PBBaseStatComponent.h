// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBaseStatComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBaseStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBaseStatComponent();

	UFUNCTION(BlueprintCallable, Category="Stat")
	bool HasStat(FName StatName) const;

	UFUNCTION(BlueprintCallable, Category="Stat")
	int32 GetStat(FName StatName) const;

	UFUNCTION(BlueprintCallable, Category="Stat")
	void SetStat(FName StatName, int32 Value);

	UFUNCTION(BlueprintCallable, Category="Stat")
	void ApplyStat(FName StatName, int32 Delta);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stat", meta=(AllowPrivateAccess="true"))
	TMap<FName, int32> Stats;
};
