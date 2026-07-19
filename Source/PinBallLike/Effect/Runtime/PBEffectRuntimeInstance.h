#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PinBallLike/Effect/Runtime/PBEffectRuntimeInterfaces.h"
#include "PBEffectRuntimeInstance.generated.h"

class AActor;
class UPBBallEffectRuntimeComponent;
class UPBBallResourceComponent;
class UPBBaseResourceComponent;
class UPBBaseStatComponent;

UCLASS(Abstract)
class PINBALLLIKE_API UPBEffectRuntimeInstanceBase : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UPBBallEffectRuntimeComponent* InOwnerComponent);
	virtual UWorld* GetWorld() const override;

	virtual IPBCollisionDamageModifier* AsCollisionDamageModifier() { return nullptr; }
	virtual IPBSkillDamageModifier* AsSkillDamageModifier() { return nullptr; }
	virtual IPBEnemyHitListener* AsEnemyHitListener() { return nullptr; }
	virtual IPBComboChangedListener* AsComboChangedListener() { return nullptr; }
	virtual IPBResourceRevivedListener* AsResourceRevivedListener() { return nullptr; }

protected:
	UPBBallEffectRuntimeComponent* GetOwnerComponent() const { return OwnerComponent.Get(); }
	AActor* GetOwnerActor() const;
	UPBBaseStatComponent* GetOwnerStatComponent() const;
	UPBBaseResourceComponent* GetOwnerResourceComponent() const;
	UPBBallResourceComponent* GetOwnerBallResourceComponent() const;

	void ApplyStatDelta(FName StatName, FName ModifyType, float Value) const;
	void ApplyTemporaryStatBuff(FName StatName, FName ModifyType, float Value, float Duration);
	void RestoreStatValue(FName StatName, int32 PreviousValue);
	AActor* FindNearestEnemy(AActor* SourceActor) const;
	void ApplyDamageToEnemy(AActor* EnemyActor, int32 DamageAmount) const;
	static int32 CalculateModifiedIntValue(int32 CurrentValue, FName ModifyType, float Value);

private:
	UPROPERTY(Transient)
	TObjectPtr<UPBBallEffectRuntimeComponent> OwnerComponent;
};

UCLASS()
class PINBALLLIKE_API UPBFirstAttackEffectRuntimeInstance
	: public UPBEffectRuntimeInstanceBase
	, public IPBCollisionDamageModifier
	, public IPBEnemyHitListener
{
	GENERATED_BODY()

public:
	void Setup(float InAttackPercent, int32 InExtraDamage);
	virtual IPBCollisionDamageModifier* AsCollisionDamageModifier() override { return this; }
	virtual IPBEnemyHitListener* AsEnemyHitListener() override { return this; }
	virtual int32 ModifyCollisionDamage(int32 CurrentDamage) override;
	virtual void HandleEnemyHit(AActor* EnemyActor) override;

private:
	float AttackPercent = 0.0f;
	int32 ExtraDamage = 0;
	bool bAttackConsumed = false;
	bool bExtraDamageConsumed = false;
};

UCLASS()
class PINBALLLIKE_API UPBComboStatBuffEffectRuntimeInstance
	: public UPBEffectRuntimeInstanceBase
	, public IPBComboChangedListener
{
	GENERATED_BODY()

public:
	void Setup(FName InStatName, FName InModifyType, float InValue, int32 InRequiredCombo);
	virtual IPBComboChangedListener* AsComboChangedListener() override { return this; }
	virtual void HandleComboChanged(int32 CurrentCombo, int32 MaxCombo) override;

private:
	FName StatName = NAME_None;
	FName ModifyType = NAME_None;
	float Value = 0.0f;
	int32 RequiredCombo = 0;
	bool bApplied = false;
};

UCLASS()
class PINBALLLIKE_API UPBSkillDamageEffectRuntimeInstance
	: public UPBEffectRuntimeInstanceBase
	, public IPBSkillDamageModifier
{
	GENERATED_BODY()

public:
	void Setup(float InPercent);
	virtual IPBSkillDamageModifier* AsSkillDamageModifier() override { return this; }
	virtual int32 ModifySkillDamage(int32 CurrentDamage) const override;

private:
	float Percent = 0.0f;
};

UCLASS()
class PINBALLLIKE_API UPBOnHitStatStackEffectRuntimeInstance
	: public UPBEffectRuntimeInstanceBase
	, public IPBEnemyHitListener
{
	GENERATED_BODY()

public:
	void Setup(FName InStatName, FName InModifyType, float InValue);
	virtual IPBEnemyHitListener* AsEnemyHitListener() override { return this; }
	virtual void HandleEnemyHit(AActor* EnemyActor) override;

private:
	FName StatName = NAME_None;
	FName ModifyType = NAME_None;
	float Value = 0.0f;
};

UCLASS()
class PINBALLLIKE_API UPBOnHitDamageIgnoreEffectRuntimeInstance
	: public UPBEffectRuntimeInstanceBase
	, public IPBEnemyHitListener
{
	GENERATED_BODY()

public:
	void Setup(FName InResourceName, int32 InIgnoreCount, float InChancePercent);
	virtual IPBEnemyHitListener* AsEnemyHitListener() override { return this; }
	virtual void HandleEnemyHit(AActor* EnemyActor) override;

private:
	FName ResourceName = NAME_None;
	int32 IgnoreCount = 0;
	float ChancePercent = 0.0f;
	bool bRolled = false;
};

UCLASS()
class PINBALLLIKE_API UPBComboExtraDamageEffectRuntimeInstance
	: public UPBEffectRuntimeInstanceBase
	, public IPBComboChangedListener
{
	GENERATED_BODY()

public:
	void Setup(int32 InComboInterval, int32 InDamageAmount);
	virtual IPBComboChangedListener* AsComboChangedListener() override { return this; }
	virtual void HandleComboChanged(int32 CurrentCombo, int32 MaxCombo) override;

private:
	int32 ComboInterval = 0;
	int32 DamageAmount = 0;
	int32 LastAppliedCombo = 0;
};

UCLASS()
class PINBALLLIKE_API UPBAttackPercentEffectRuntimeInstance
	: public UPBEffectRuntimeInstanceBase
	, public IPBCollisionDamageModifier
{
	GENERATED_BODY()

public:
	void Setup(float InPercent);
	virtual IPBCollisionDamageModifier* AsCollisionDamageModifier() override { return this; }
	virtual int32 ModifyCollisionDamage(int32 CurrentDamage) override;

private:
	float Percent = 0.0f;
};

UCLASS()
class PINBALLLIKE_API UPBReviveStatBuffEffectRuntimeInstance
	: public UPBEffectRuntimeInstanceBase
	, public IPBResourceRevivedListener
{
	GENERATED_BODY()

public:
	void Setup(FName InStatName, FName InModifyType, float InValue);
	virtual IPBResourceRevivedListener* AsResourceRevivedListener() override { return this; }
	virtual void HandleResourceRevived(FName ResourceName, float ReviveValue) override;

private:
	FName StatName = NAME_None;
	FName ModifyType = NAME_None;
	float Value = 0.0f;
};

UCLASS()
class PINBALLLIKE_API UPBFirstHitTimedStatBuffEffectRuntimeInstance
	: public UPBEffectRuntimeInstanceBase
	, public IPBEnemyHitListener
{
	GENERATED_BODY()

public:
	void Setup(FName InStatName, FName InModifyType, float InValue, float InDuration);
	virtual IPBEnemyHitListener* AsEnemyHitListener() override { return this; }
	virtual void HandleEnemyHit(AActor* EnemyActor) override;

private:
	FName StatName = NAME_None;
	FName ModifyType = NAME_None;
	float Value = 0.0f;
	float Duration = 0.0f;
	bool bApplied = false;
};

UCLASS()
class PINBALLLIKE_API UPBSwitchNextHitDamageEffectRuntimeInstance
	: public UPBEffectRuntimeInstanceBase
	, public IPBCollisionDamageModifier
{
	GENERATED_BODY()

public:
	void Setup(float InAttackPercent);
	virtual IPBCollisionDamageModifier* AsCollisionDamageModifier() override { return this; }
	virtual int32 ModifyCollisionDamage(int32 CurrentDamage) override;

private:
	float AttackPercent = 0.0f;
	bool bConsumed = false;
};

UCLASS()
class PINBALLLIKE_API UPBSwitchTimedStatBuffEffectRuntimeInstance : public UPBEffectRuntimeInstanceBase
{
	GENERATED_BODY()

public:
	void Setup(FName InStatName, FName InModifyType, float InValue, float InDuration);
	void ApplyNow();

private:
	FName StatName = NAME_None;
	FName ModifyType = NAME_None;
	float Value = 0.0f;
	float Duration = 0.0f;
	bool bApplied = false;
};
