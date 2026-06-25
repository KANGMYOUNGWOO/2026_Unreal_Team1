// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PBBallComboComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBallComboChanged, int32, CurrentCombo, int32, MaxCombo);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallComboComponent : public UActorComponent, public IComboable
{
	GENERATED_BODY()

public:
	UPBBallComboComponent();

	UPROPERTY(BlueprintAssignable, Category = "Ball|Combo")
	FOnBallComboChanged OnComboChanged;

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	virtual int32 GetCombo() const override;

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	int32 GetMaxCombo() const;

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	virtual void SetCombo(int32 Value) override;

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	virtual void AddCombo(int32 Delta) override;

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	virtual bool TryConsumeCombo(int32 Cost) override;

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	virtual void ResetCombo() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Ball|Combo")
	int32 CurrentCombo = 0;

	UPROPERTY(VisibleAnywhere, Category = "Ball|Combo")
	int32 MaxCombo = 0;
};
