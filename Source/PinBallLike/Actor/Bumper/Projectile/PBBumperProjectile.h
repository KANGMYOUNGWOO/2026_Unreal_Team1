// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Projectile/ProjectileBase.h"
#include "PBBumperProjectile.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class EPBBumperProjectilePayload : uint8
{
	None,
	BossDamage,
	BossGroggy,
	BossVulnerability
};

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FPBBumperProjectileResolvedSignature,
	class APBBumperProjectile*,
	bool);

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBumperProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:
	APBBumperProjectile();

	static APBBumperProjectile* SpawnForTarget(
		UObject* WorldContext,
		TSubclassOf<APBBumperProjectile> InProjectileClass,
		AActor* OwnerActor,
		const FVector& SpawnLocation,
		const FRotator& SpawnRotation,
		AActor* InTargetActor,
		EPBBumperProjectilePayload InPayload,
		int32 InPower,
		float InPayloadDuration,
		float InLifetime,
		UNiagaraSystem* InDeliveryVfx = nullptr,
		UNiagaraSystem* InImpactVfx = nullptr,
		UNiagaraSystem* InStatusVfx = nullptr);

	void ConfigureForTarget(
		AActor* InTargetActor,
		EPBBumperProjectilePayload InPayload,
		int32 InPower,
		bool bInDestroyOnResolved,
		float InPayloadDuration = 0.0f,
		UNiagaraSystem* InDeliveryVfx = nullptr,
		UNiagaraSystem* InImpactVfx = nullptr,
		UNiagaraSystem* InStatusVfx = nullptr);
	void ResetForPool();

	FPBBumperProjectileResolvedSignature OnProjectileResolved;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void HandleProjectileBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Projectile",
		meta = (ClampMin = "0.0", Units = "cm/s^2"))
	float HomingAcceleration = 8000.0f;

private:
	bool ApplyPayload(AActor* Target, int32& OutAppliedDamage) const;
	void BroadcastDamageLog(int32 AppliedDamage, const FVector& HitLocation) const;
	void StartDeliveryVfx();
	void StopDeliveryVfx();
	void PlayResolvedVfx(AActor* Target) const;

	TWeakObjectPtr<AActor> TargetActor;
	EPBBumperProjectilePayload Payload = EPBBumperProjectilePayload::None;
	int32 PayloadPower = 0;
	float PayloadDuration = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> DeliveryVfx;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> ImpactVfx;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> StatusVfx;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> DeliveryVfxComponent;

	bool bDestroyOnResolved = true;
	bool bHasResolved = false;
};
