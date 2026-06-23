// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/PBBumperBase.h"
#include "PBCollisionBumper.generated.h"

class ABallBase;
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

	// 이 태그를 가진 PrimitiveComponent를 충돌 판정 영역으로 자동 등록한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Collision")
	FName CollisionAreaTag = TEXT("BumperCollision");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Collision")
	TArray<TObjectPtr<UPrimitiveComponent>> CollisionAreas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Collision", meta = (ClampMin = "0.0", Units = "cm/s"))
	float BounceVelocityStrength = 1200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Reaction")
	TObjectPtr<UPBBumperReactionComponent> ReactionComponent;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Collision")
	void RegisterTaggedCollisionAreas();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Collision")
	void RegisterCollisionArea(UPrimitiveComponent* CollisionArea);

#pragma region Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Collision")
	void OnCollisionBumperHit(ABallBase* Ball, const FHitResult& Hit);
#pragma endregion

private:
	// 충돌 노멀 방향으로 볼에 추가 속도를 부여한다.
	void AddBounceVelocityToBall(ABallBase* Ball, const FHitResult& Hit) const;

	UFUNCTION()
	void HandleComponentHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);
};
