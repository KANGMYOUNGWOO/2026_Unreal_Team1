// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PBBallComboComponent.generated.h"

class APBBattleGameState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallComboChanged, int32, CurrentCombo);

UCLASS(ClassGroup=(PinBall), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallComboComponent : public UActorComponent, public IComboable
{
	GENERATED_BODY()

public:
	UPBBallComboComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintAssignable, Category = "Ball|Combo")
	FOnBallComboChanged OnComboChanged;

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	virtual int32 GetCombo() const override;

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	virtual void SetCombo(int32 Value) override;

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	virtual void AddCombo(int32 Delta) override;

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	virtual bool TryConsumeCombo(int32 Cost) override;

	UFUNCTION(BlueprintCallable, Category = "Ball|Combo")
	virtual void ResetCombo() override;

private:
	APBBattleGameState* GetBattleGameState() const;

	UFUNCTION()
	void HandleBattleComboChanged(int32 CurrentCombo);
	
};
