// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBallComboComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBallComboChanged, int32, CurrentCombo, int32, MaxCombo);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBallComboComponent();

	UPROPERTY(BlueprintAssignable, Category = "Ball|Combo")
	FOnBallComboChanged OnComboChanged;

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	int32 GetCombo() const;

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	int32 GetMaxCombo() const;

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	void SetCombo(int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	void AddCombo(int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	bool TryConsumeCombo(int32 Cost);

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	void ResetCombo();

private:
	UPROPERTY(VisibleAnywhere, Category = "Ball|Combo")
	int32 CurrentCombo = 0;

	UPROPERTY(VisibleAnywhere, Category = "Ball|Combo")
	int32 MaxCombo = 0;

	void BroadcastComboChanged();
};
