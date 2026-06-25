// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/PBBumperBase.h"
#include "PBCollisionBumper.generated.h"

class APBBallBase;
class UPBBumperReactionComponent;
class UPrimitiveComponent;

UCLASS()
class PINBALLLIKE_API APBCollisionBumper : public APBBumperBase
{
	GENERATED_BODY()

public:
	APBCollisionBumper();

protected:
	virtual void BeginPlay() override;

	// 이 태그를 가진 PrimitiveComponent를 실제 충돌 영역으로 자동 등록한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Collision")
	FName CollisionAreaTag = TEXT("BumperCollision");

	// 이 태그를 가진 PrimitiveComponent 안에서 발생한 충돌만 범퍼 발동으로 처리한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Collision")
	FName TriggerAreaTag = TEXT("BumperTrigger");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Collision")
	TArray<TObjectPtr<UPrimitiveComponent>> CollisionAreas;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Collision")
	TArray<TObjectPtr<UPrimitiveComponent>> TriggerAreas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Collision", meta = (ClampMin = "0.0", Units = "cm/s"))
	float BounceVelocityStrength = 1200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Reaction")
	TObjectPtr<UPBBumperReactionComponent> ReactionComponent;

#pragma region Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Collision")
	void OnCollisionBumperHit(APBBallBase* Ball, const FHitResult& Hit);
#pragma endregion

private:
	TMap<TWeakObjectPtr<APBBallBase>, int32> TriggeringBallOverlapCounts;

	void RegisterTaggedAreas();
	void SetupCollisionArea(UPrimitiveComponent* CollisionArea);
	void SetupTriggerArea(UPrimitiveComponent* TriggerArea);

	// 충돌 노멀 방향으로 볼에 추가 속도를 부여한다.
	void AddBounceVelocityToBall(APBBallBase* Ball, const FHitResult& Hit) const;

	bool IsBallInTriggerArea(APBBallBase* Ball) const;

	UFUNCTION()
	void HandleComponentHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleTriggerEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
};
