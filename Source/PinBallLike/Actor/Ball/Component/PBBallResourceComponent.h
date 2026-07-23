#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PBBallResourceComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FPBOnBallResourceRevived, FName, float);

UENUM(BlueprintType)
enum class EPBBallResourceDamageRuleKind : uint8
{
	DamageIgnore,
	ReviveOnZero,
	PostDamageHeal
};

USTRUCT(BlueprintType)
struct FPBBallResourceDamageRule
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EPBBallResourceDamageRuleKind Kind = EPBBallResourceDamageRuleKind::DamageIgnore;

	UPROPERTY(BlueprintReadOnly)
	FName ResourceName = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	int32 Count = 0;

	UPROPERTY(BlueprintReadOnly)
	float Value = 0.0f;
};

UCLASS(ClassGroup=(PinBall), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallResourceComponent : public UPBBaseResourceComponent
{
	GENERATED_BODY()

public:
	FPBOnBallResourceRevived OnResourceRevived;

	virtual void TakeDamage(int32 Damage) override;

	void TakeSelfCollisionDamage(int32 Damage);

	UFUNCTION(BlueprintCallable, Category="Ball|Resource|Damage")
	void AddDamageIgnoreCount(FName ResourceName, int32 Count);

	UFUNCTION(BlueprintCallable, Category="Ball|Resource|Damage")
	int32 GetDamageIgnoreCount(FName ResourceName) const;

	UFUNCTION(BlueprintCallable, Category="Ball|Resource|Damage")
	void AddReviveOnZeroCount(FName ResourceName, int32 Count, float ReviveValue);

	UFUNCTION(BlueprintCallable, Category="Ball|Resource|Damage")
	void AddPostDamageHealCount(FName ResourceName, int32 Count, float HealValue);

private:
	void TakeDamageInternal(int32 Damage, bool bResetComboOnAppliedDamage);
	void ResetComboAfterAppliedDamage() const;
	void AddResourceDamageRule(EPBBallResourceDamageRuleKind Kind, FName ResourceName, int32 Count, float Value);
	FPBBallResourceDamageRule* FindResourceDamageRule(EPBBallResourceDamageRuleKind Kind, FName ResourceName);
	const FPBBallResourceDamageRule* FindResourceDamageRule(EPBBallResourceDamageRuleKind Kind, FName ResourceName) const;
	bool ConsumeResourceDamageRule(EPBBallResourceDamageRuleKind Kind, FName ResourceName, float& OutValue);
	bool ConsumeDamageIgnoreCount(FName ResourceName);
	bool TryReviveOnZero(FName ResourceName);
	void TryApplyPostDamageHeal(FName ResourceName, float PreviousCurrent);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ball|Resource|Damage", meta=(AllowPrivateAccess="true"))
	TArray<FPBBallResourceDamageRule> DamageRules;
};
