#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBGolemBossTypes.h"
#include "PBGolemBossHand.generated.h"

class APBGolemBoss;
class UPBGolemHandMovementComponent;
class USceneComponent;
class USkeletalMeshComponent;

UCLASS()
class PINBALLLIKE_API APBGolemBossHand : public AActor
{
	GENERATED_BODY()

public:
	APBGolemBossHand();

	virtual void Tick(float DeltaSeconds) override;

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

protected:
	void MoveToWorldLocationInternal(FVector TargetWorldLocation, float Duration, bool IsIgnorePatternMovementLock);
	FVector CalculateActorTargetLocationForTelegraphStart(FVector TargetWorldLocation) const;
	void UpdateFistAimRotation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Golem Hand")
	void BP_OnFistLaunchStarted(FVector TargetWorldLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Golem Hand Animation")
	void BP_OnIdleAnimationSyncRequested(float SyncedPosition);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand")
	TObjectPtr<USkeletalMeshComponent> HandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand")
	TObjectPtr<USceneComponent> TelegraphStartPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand")
	TObjectPtr<UPBGolemHandMovementComponent> HandMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand")
	EPBGolemBossHandType HandType = EPBGolemBossHandType::Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Hand Animation", meta = (ClampMin = "0.1"))
	float IdleAnimationLength = 1.0f;

	UPROPERTY(Transient)
	TObjectPtr<APBGolemBoss> OwnerBoss;

	UPROPERTY(Transient)
	TObjectPtr<AActor> FistAimTargetActor;

	bool IsFistAiming = false;
	bool IsPatternMovementLocked = false;
};
