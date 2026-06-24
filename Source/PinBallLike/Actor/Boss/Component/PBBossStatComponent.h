#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PBBossStatComponent.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPBBossHPChangedSignature, int32, HP, int32, MaxHP);

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBossStatComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Damage")
	void ApplyBossDamage(FName HitPointName, int32 DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	bool IsDead() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat", meta = (ClampMin = "1"))
	int32 MaxHP = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Stat")
	int32 HP = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
	bool IsEnraged = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage", meta = (ClampMin = "0"))
	int32 DefaultHPDamageMultiplierPercent = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage")
	TMap<FName, FBossGroggyPointData> HitPointDataMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|State", meta = (ClampMin = "0", ClampMax = "100"))
	int32 EnrageHPRatioPercent = 40;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Stat")
	FPBBossHPChangedSignature OnHPChanged;

private:
	int32 GetHPDamageMultiplierPercent(FName HitPointName) const;
	bool IsEnrageThresholdReached() const;
	bool CanNotifyOwner() const;

	UPROPERTY(Transient)
	TObjectPtr<AActor> OwnerActor;
};
