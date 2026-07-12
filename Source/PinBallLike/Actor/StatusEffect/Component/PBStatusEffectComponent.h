// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBStatusEffectComponent.generated.h"

class AActor;
class UPBBaseStatusEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPBStatusEffectChangedSignature, FName, StatusEffectId, int32, StackCount);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBStatusEffectComponent();

	UPROPERTY(BlueprintAssignable, Category = "StatusEffect")
	FPBStatusEffectChangedSignature OnStatusEffectApplied;

	UPROPERTY(BlueprintAssignable, Category = "StatusEffect")
	FPBStatusEffectChangedSignature OnStatusEffectRemoved;

	UFUNCTION(BlueprintCallable, Category = "StatusEffect")
	bool ApplyStatusEffect(FName StatusEffectId);

	UFUNCTION(BlueprintCallable, Category = "StatusEffect")
	bool RemoveStatusEffect(FName StatusEffectId);

	UFUNCTION(BlueprintCallable, Category = "StatusEffect")
	void RemoveAllStatusEffects();

	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	bool HasStatusEffect(FName StatusEffectId) const;

	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	bool HasEffectTag(FName Tag) const;

	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	int32 GetStatusEffectStackCount(FName StatusEffectId) const;

	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	UPBBaseStatusEffect* GetStatusEffect(FName StatusEffectId) const;

	UFUNCTION(BlueprintCallable, Category = "StatusEffect")
	void GetActiveStatusEffects(TArray<UPBBaseStatusEffect*>& OutStatusEffects) const;

	UFUNCTION(BlueprintCallable, Category = "StatusEffect")
	void NotifyStatusEffectEvent(FName TriggerEvent);
	
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPBBaseStatusEffect* CreateStatusEffect(FName StatusEffectId);
	int32 FindStatusEffectIndex(FName StatusEffectId) const;
	void RemoveExpiredEffects();
	void RemoveStatusEffectAt(int32 Index, bool bExpired);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffect", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UPBBaseStatusEffect>> ActiveStatusEffects;
};
