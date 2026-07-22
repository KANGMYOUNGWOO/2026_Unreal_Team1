#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBallEffectRuntimeComponent.generated.h"

class AActor;
class IPBCollisionDamageModifier;
class IPBSkillDamageModifier;
class IPBEnemyHitListener;
class IPBComboChangedListener;
class IPBResourceRevivedListener;
class UPBEffectRuntimeInstanceBase;

UCLASS(ClassGroup=(PinBall), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallEffectRuntimeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBallEffectRuntimeComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void AddFirstAttackRule(float AttackPercent, int32 ExtraDamage);
	void AddComboStatBuffRule(FName StatName, FName ModifyType, float Value, int32 RequiredCombo);
	void AddComboPeriodStatBuffRule(FName StatName, FName ModifyType, float Value, int32 RequiredCombo);
	void AddSkillDamagePercent(float Percent);
	void AddOnHitStatStackRule(FName StatName, FName ModifyType, float Value);
	void AddOnHitDamageIgnoreChance(FName ResourceName, int32 IgnoreCount, float ChancePercent);
	void AddComboExtraDamageRule(int32 ComboInterval, int32 DamageAmount);
	void AddPendingMultiBallCount(int32 Count);
	void AddPierceRule(float DamageRetentionPercent);
	void AddAttackPercent(float Percent);
	void AddReviveStatBuffRule(FName StatName, FName ModifyType, float Value);
	void AddFirstHitTimedStatBuffRule(FName StatName, FName ModifyType, float Value, float Duration);
	void AddSwitchNextHitDamageRule(float AttackPercent);
	void AddSwitchTimedStatBuffRule(FName StatName, FName ModifyType, float Value, float Duration);
	
	
	
	int32 ModifyCollisionDamage(int32 BaseDamage);
	int32 ModifySkillDamage(int32 BaseDamage) const;
	void HandleEnemyHit(AActor* EnemyActor);
	void HandleComboChanged(int32 CurrentCombo);

	UFUNCTION(BlueprintPure, Category = "Effect|Ball")
	int32 GetPendingMultiBallCount() const { return PendingMultiBallCount; }

	UFUNCTION(BlueprintPure, Category = "Effect|Ball")
	bool HasPierce() const { return bPierceGranted; }

	UFUNCTION(BlueprintPure, Category = "Effect|Ball")
	float GetPierceDamageRetentionPercent() const { return PierceDamageRetentionPercent; }

private:
	void RegisterRuntimeInstance(UPBEffectRuntimeInstanceBase* Instance);
	void HandleResourceRevived(FName ResourceName, float ReviveValue);

	int32 PendingMultiBallCount = 0;
	bool bPierceGranted = false;
	float PierceDamageRetentionPercent = 0.0f;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBEffectRuntimeInstanceBase>> ActiveEffectInstances;

	TArray<IPBCollisionDamageModifier*> CollisionDamageModifiers;
	TArray<IPBSkillDamageModifier*> SkillDamageModifiers;
	TArray<IPBEnemyHitListener*> EnemyHitListeners;
	TArray<IPBComboChangedListener*> ComboChangedListeners;
	TArray<IPBResourceRevivedListener*> ResourceRevivedListeners;
};
