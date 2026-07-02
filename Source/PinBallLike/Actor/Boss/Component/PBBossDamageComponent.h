#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBossHitPartComponent.h"
#include "PBBossDamageComponent.generated.h"

class APBBossBase;
class AActor;
class UPrimitiveComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FPBBossDamageSourceHitAppliedSignature, AActor*, const FHitResult&);

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBossDamageComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Damage")
	void ApplyHitPartDamage(AActor* DamageSource, UPrimitiveComponent* HitComponent, int32 DamageAmount, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category = "Boss|Damage")
	void SetPinballCollisionDamageBlocked(bool IsBlocked);

	UFUNCTION(BlueprintPure, Category = "Boss|Damage")
	bool IsPinballCollisionDamageBlocked() const;

	FPBBossDamageSourceHitAppliedSignature OnDamageSourceHitApplied;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage")
	FName DefaultHitPointName = TEXT("Normal");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Damage")
	bool IsPinballCollisionDamageBlockedValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage", meta = (ClampMin = "0"))
	int32 MaxDamageCountPerFrame = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage", meta = (ClampMin = "0"))
	float DamageCooldownSeconds = 0.25f;

private:
	struct FPBBossHitPartInfo
	{
		EPBBossHitPartType HitPartType = EPBBossHitPartType::Body;
		FName HitPointName = TEXT("Normal");
	};

	FPBBossHitPartInfo ResolveHitPartInfo(const UPrimitiveComponent* HitComponent, const FHitResult& Hit) const;
	const UPBBossHitPartComponent* FindHitPartComponent(const UPrimitiveComponent* HitComponent) const;
	const UPBBossHitPartComponent* FindHitPartComponent(FName HitPointName) const;
	bool IsDamageBlocked(AActor* DamageSource) const;
	bool CanApplyDamage(FName HitPointName, int32 DamageAmount) const;
	bool CanApplyDamageRateLimit() const;
	bool IsWeakPointHitBlocked(const FPBBossHitPartInfo& HitPartInfo) const;
	void ApplyResolvedDamage(AActor* DamageSource, const FPBBossHitPartInfo& HitPartInfo, int32 DamageAmount, const FHitResult& Hit);
	void ApplyDamageToBoss(FName HitPointName, int32 DamageAmount);
	void RecordDamageRateLimit();

	UPROPERTY(Transient)
	TObjectPtr<APBBossBase> OwnerBoss;

	float LastDamageTimeSeconds = -1.0f;
	uint64 LastDamageFrameNumber = 0;
	int32 CurrentFrameDamageCount = 0;
};
