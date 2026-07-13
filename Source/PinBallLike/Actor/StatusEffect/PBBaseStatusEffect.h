// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectModifierRow.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectRow.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectTriggerRow.h"
#include "UObject/Object.h"
#include "PBBaseStatusEffect.generated.h"

class AActor;
class UPBStatusEffectComponent;

/**
 * Runtime status effect instance. Subclasses own actual gameplay behavior;
 * the component only manages lifetime and forwards events.
 */
UCLASS(BlueprintType)
class PINBALLLIKE_API UPBBaseStatusEffect : public UObject
{
	GENERATED_BODY()

public:
	void InitializeStatusEffect(
		UPBStatusEffectComponent* InOwnerComponent,
		const FPBStatusEffectRow& InStatusEffectRow,
		const TArray<FPBStatusEffectModifierRow>& InModifierRows,
		const TArray<FPBStatusEffectTriggerRow>& InTriggerRows);

	FName GetStatusEffectId() const { return StatusEffectRow.StatusEffectId; }
	int32 GetStackCount() const { return StackCount; }
	const FPBStatusEffectRow& GetStatusEffectRow() const { return StatusEffectRow; }
	UPBStatusEffectComponent* GetOwnerComponent() const { return OwnerComponent.Get(); }
	AActor* GetOwnerActor() const;

	bool HasTag(FName Tag) const;

	void RefreshDuration();
	bool IsExpired() const;
	void TickStatusEffect(float DeltaTime);
	void AddStack();

	virtual void ExecuteStatusEffect(FName TriggerEvent);

protected:
	UPROPERTY()
	TWeakObjectPtr<UPBStatusEffectComponent> OwnerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffect")
	FPBStatusEffectRow StatusEffectRow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffect")
	TArray<FPBStatusEffectModifierRow> ModifierRows;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffect")
	TArray<FPBStatusEffectTriggerRow> TriggerRows;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffect")
	int32 StackCount = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffect")
	float RemainingDuration = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffect")
	float IntervalElapsed = 0.0f;
};
