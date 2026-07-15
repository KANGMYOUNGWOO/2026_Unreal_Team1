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
	void ApplyHitPartDamage(int32 DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Boss|Damage")
	void SetPinballCollisionDamageBlocked(bool IsBlocked);

	UFUNCTION(BlueprintPure, Category = "Boss|Damage")
	bool IsPinballCollisionDamageBlocked() const;

	void ConfigureDamageSettings(FName NewDefaultHitPointName, float NewDamageCooldownSeconds);

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
	UFUNCTION()
	void HandleHitPartComponentHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	const UPBBossHitPartComponent* FindHitPartComponent(const UPrimitiveComponent* HitComponent) const;
	void BindHitPartCollisionEvents();
	bool CanApplyDamage(FName HitPointName, int32 DamageAmount) const;
	FName ResolveHitPointName() const;
	bool CanApplyDamageRateLimit() const;
	bool IsWeakPointHitBlocked(FName HitPointName) const;
	void ApplyDamageToBoss(FName HitPointName, int32 DamageAmount);
	void RecordDamageRateLimit();

	UPROPERTY(Transient)
	TObjectPtr<APBBossBase> OwnerBoss;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UPrimitiveComponent>, TObjectPtr<UPBBossHitPartComponent>> HitPartByCollisionComponent;

	float LastDamageTimeSeconds = -1.0f;
	uint64 LastDamageFrameNumber = 0;
	uint64 LastHitFrameNumber = 0;
	int32 CurrentFrameDamageCount = 0;
	FName LastHitPointName = NAME_None;
};
