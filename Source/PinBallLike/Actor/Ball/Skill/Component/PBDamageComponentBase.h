#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBDamageComponentBase.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBDamageAppliedSignature,
	AActor*, Target,
	int32, DamageAmount);

UCLASS(Abstract, ClassGroup=(PinBall))
class PINBALLLIKE_API UPBDamageComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBDamageComponentBase();

	UFUNCTION(BlueprintPure, Category = "Damage")
	bool CanApplyDamage(AActor* Target, int32 DamageAmount) const;

	UFUNCTION(BlueprintCallable, Category = "Damage")
	bool ApplyDamage(AActor* Target, int32 DamageAmount);

	void SetGroggyAmount(int32 InGroggyAmount);

	UPROPERTY(BlueprintAssignable, Category = "Damage")
	FPBDamageAppliedSignature OnDamageApplied;

private:
	void AddComboForSkillDamage() const;

	int32 GroggyAmount = 0;
};
