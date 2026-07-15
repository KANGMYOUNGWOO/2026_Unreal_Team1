#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBGolemHandMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBGolemHandMoveFinishedSignature);

UENUM(BlueprintType)
enum class EPBGolemHandMovementMode : uint8
{
	Idle,
	Autonomous,
	Command
};

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBGolemHandMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBGolemHandMovementComponent();

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand Movement")
	void InitializeHandMovement(AActor* NewBossActor, FVector NewDefaultOffset);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand Movement")
	void MoveToOffset(FVector TargetOffset, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand Movement")
	void MoveToWorldLocation(FVector TargetWorldLocation, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand Movement")
	void ReturnToDefaultOffset(float Duration);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand Movement")
	void StartAutonomousMove();

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand Movement")
	void StopAutonomousMove();

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand Movement")
	void StopMove();

	UFUNCTION(BlueprintPure, Category = "Boss|Golem Hand Movement")
	bool IsMoving() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Golem Hand Movement")
	EPBGolemHandMovementMode GetMovementMode() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Golem Hand Movement")
	FVector GetDefaultOffset() const;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Golem Hand Movement")
	FPBGolemHandMoveFinishedSignature OnMoveFinished;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Hand Movement", meta = (ClampMin = "0"))
	float AutonomousMoveRadius = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Hand Movement", meta = (ClampMin = "0.1"))
	float AutonomousMoveDurationMin = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Hand Movement", meta = (ClampMin = "0.1"))
	float AutonomousMoveDurationMax = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Hand Movement", meta = (ClampMin = "0"))
	float AutonomousWaitTimeMin = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Hand Movement", meta = (ClampMin = "0"))
	float AutonomousWaitTimeMax = 0.6f;

private:
	FVector ConvertOffsetToWorldLocation(FVector Offset) const;
	void StartMove(FVector TargetOffset, float Duration, EPBGolemHandMovementMode NewMovementMode);
	void StartMoveToWorldLocation(FVector TargetWorldLocation, float Duration, EPBGolemHandMovementMode NewMovementMode);
	void UpdateCurrentMove(float DeltaTime);
	void UpdateAutonomousMove(float DeltaTime);
	void SelectNextAutonomousMove();
	void FinishMove();

	UPROPERTY(Transient)
	TObjectPtr<AActor> BossActor;

	FVector DefaultOffset = FVector::ZeroVector;
	FVector StartLocation = FVector::ZeroVector;
	FVector TargetLocation = FVector::ZeroVector;
	float MoveDuration = 0.0f;
	float ElapsedMoveTime = 0.0f;
	float AutonomousWaitTime = 0.0f;
	EPBGolemHandMovementMode MovementMode = EPBGolemHandMovementMode::Idle;
	bool IsMovingValue = false;
};
