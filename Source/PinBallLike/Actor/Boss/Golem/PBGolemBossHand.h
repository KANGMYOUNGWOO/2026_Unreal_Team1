#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PBGolemBossTypes.h"
#include "PBGolemBossHand.generated.h"

class APBGolemBoss;
class UPBBossHitEffectComponent;
class UPBGolemHandMovementComponent;
class USceneComponent;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBGolemHandHPChangedSignature,
	int32, CurrentHandHP,
	int32, MaxHandHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBGolemHandStateChangedSignature);

UCLASS()
class PINBALLLIKE_API APBGolemBossHand : public AActor, public IBossInterface
{
	GENERATED_BODY()

public:
	APBGolemBossHand();

	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool DamageToBoss_Implementation(int32 DamageAmount) override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand")
	void InitializeGolemHand(APBGolemBoss* NewOwnerBoss, EPBGolemBossHandType NewHandType, FVector NewDefaultOffset);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand")
	void MoveToOffset(FVector TargetOffset, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand")
	void MoveToWorldLocation(FVector TargetWorldLocation, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand")
	void MoveTelegraphStartToWorldLocation(FVector TargetWorldLocation, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand")
	void MoveToWorldTransform(FTransform TargetTransform, float Duration);

	void MoveTelegraphStartToWorldLocationForPattern(FVector TargetWorldLocation, float Duration);
	void MoveToWorldTransformForPattern(FTransform TargetTransform, float Duration);
	void BeginPatternMovementLock();
	void EndPatternMovementLock();

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand")
	void ReturnToDefaultOffset(float Duration);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand")
	void LaunchFistAtActor(AActor* TargetActor, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand")
	void LaunchFistAtLocation(FVector TargetWorldLocation, float Duration);

	void LaunchFistAtLocationForPattern(FVector TargetWorldLocation, FVector Direction, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand")
	void StartFistAimAtActor(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand")
	void StopFistAim();

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand")
	void StartAutonomousMove();

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand")
	void StopAutonomousMove();

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand Animation")
	void RequestIdleAnimationSync();

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand Animation")
	void SetIsPunching(bool NewIsPunching);

	UFUNCTION(BlueprintPure, Category = "Boss|Golem Hand Animation")
	float GetPunchAlpha() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Golem Hand Animation")
	float GetSyncedIdleAnimationPosition() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Golem Hand")
	EPBGolemBossHandType GetHandType() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Golem Hand")
	UPBGolemHandMovementComponent* GetHandMovementComponent() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Golem Hand")
	USkeletalMeshComponent* GetHandMeshComponent() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Golem Hand")
	USceneComponent* GetTelegraphStartComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand|HP")
	void ApplyHandDamage(int32 DamageAmount);

	UFUNCTION(BlueprintPure, Category = "Boss|Golem Hand|HP")
	bool IsHandAvailable() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Golem Hand|HP")
	int32 GetCurrentHandHP() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Golem Hand|HP")
	int32 GetMaxHandHP() const;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Golem Hand|HP")
	FPBGolemHandHPChangedSignature OnHandHPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Golem Hand|HP")
	FPBGolemHandStateChangedSignature OnHandDestroyed;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Golem Hand|HP")
	FPBGolemHandStateChangedSignature OnHandRegenerated;

protected:
	void MoveToWorldLocationInternal(FVector TargetWorldLocation, float Duration, bool IsIgnorePatternMovementLock);
	FVector CalculateActorTargetLocationForTelegraphStart(FVector TargetWorldLocation) const;
	void UpdateFistAimRotation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Golem Hand")
	void BP_OnFistLaunchStarted(FVector TargetWorldLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Golem Hand Animation")
	void BP_OnIdleAnimationSyncRequested(float SyncedPosition);

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Golem Hand|HP")
	void BP_OnHandDestroyed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Golem Hand|HP")
	void BP_OnHandRegenerated();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand")
	TObjectPtr<USkeletalMeshComponent> HandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand")
	TObjectPtr<USceneComponent> TelegraphStartPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand")
	TObjectPtr<UPBGolemHandMovementComponent> HandMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand")
	TObjectPtr<UPBBossHitEffectComponent> HitEffectComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand")
	EPBGolemBossHandType HandType = EPBGolemBossHandType::Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Hand Animation", meta = (ClampMin = "0.1"))
	float IdleAnimationLength = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand Animation")
	bool IsPunching = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Hand|HP", meta = (ClampMin = "1"))
	int32 MaxHandHP = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand|HP")
	int32 CurrentHandHP = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Hand|HP", meta = (ClampMin = "0.0"))
	float RegenerationDelay = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Hand|HP", meta = (ClampMin = "0"))
	int32 DestroyedGroggyAmount = 25;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand|HP")
	bool IsHandDestroyedValue = false;

	UPROPERTY(Transient)
	TObjectPtr<APBGolemBoss> OwnerBoss;

	UPROPERTY(Transient)
	TObjectPtr<AActor> FistAimTargetActor;

	void DestroyHand();
	void RegenerateHand();
	void SetHandActive(bool IsActive);

	FTimerHandle RegenerationTimerHandle;
	bool IsFistAiming = false;
	bool IsPatternMovementLocked = false;
};
