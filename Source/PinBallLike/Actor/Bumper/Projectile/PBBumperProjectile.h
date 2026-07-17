// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Projectile/ProjectileBase.h"
#include "PBBumperProjectile.generated.h"

/** 범퍼 투사체가 보스에게 전달할 실제 명중 효과입니다. */
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

/**
 * 범퍼와 소환 포탑이 함께 사용하는 보스 표적 유도탄입니다.
 * 시트 Power는 발사 시 payload로 보관되고, 보스와 실제로 겹쳤을 때만 적용됩니다.
 */
UCLASS(Blueprintable)
class PINBALLLIKE_API APBBumperProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:
	APBBumperProjectile();

	/** 범퍼와 지연 효과가 같은 생성/설정 절차를 공유하도록 유도탄을 한 번에 생성합니다. */
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
		float InLifetime);

	void ConfigureForTarget(
		AActor* InTargetActor,
		EPBBumperProjectilePayload InPayload,
		int32 InPower,
		bool bInDestroyOnResolved,
		float InPayloadDuration = 0.0f);
	void ResetForPool();

	FPBBumperProjectileResolvedSignature OnProjectileResolved;

protected:
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
	bool ApplyPayload(AActor* Target) const;

	TWeakObjectPtr<AActor> TargetActor;
	EPBBumperProjectilePayload Payload = EPBBumperProjectilePayload::None;
	int32 PayloadPower = 0;
	float PayloadDuration = 0.0f;
	bool bDestroyOnResolved = true;
	bool bHasResolved = false;
};
