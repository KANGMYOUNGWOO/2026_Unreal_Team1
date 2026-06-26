#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectKey.h"
#include "PBBossProjectile.generated.h"

class APBBallBase;
class UPrimitiveComponent;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class PINBALLLIKE_API APBBossProjectile : public AActor
{
	GENERATED_BODY()

public:
	// 투사체 충돌체와 이동 컴포넌트를 생성합니다.
	APBBossProjectile();

	UFUNCTION(BlueprintCallable, Category = "Boss|Projectile")
	// 투사체 이동 속도를 설정하고 이동 컴포넌트에 반영합니다.
	void SetProjectileSpeed(float NewProjectileSpeed);

	UFUNCTION(BlueprintPure, Category = "Boss|Projectile")
	// 현재 투사체 이동 속도를 반환합니다.
	float GetProjectileSpeed() const;

protected:
	// 게임 시작 시 충돌 설정과 초기 겹침 검사를 수행합니다.
	virtual void BeginPlay() override;

	UFUNCTION()
	// 투사체가 다른 액터와 겹쳤을 때 핀볼 충돌 처리를 수행합니다.
	void HandleProjectileBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Projectile")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (ClampMin = "0"))
	float ProjectileSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (ClampMin = "0.1"))
	float LifeTimeSeconds = 3.0f;

private:
	// ProjectileMovementComponent에 현재 속도 값을 적용합니다.
	void ApplyProjectileSpeed();
	// 투사체 충돌 이벤트와 충돌 프로필을 설정합니다.
	void SetProjectileCollision();
	// 생성 직후 이미 겹쳐 있는 핀볼을 검사합니다.
	void CheckInitialOverlappingPinballs();
	// 핀볼과 겹쳤을 때 중복 처리 방지 후 효과를 적용합니다.
	void ProcessPinballOverlap(APBBallBase* Pinball);

	TSet<TObjectKey<APBBallBase>> ProcessedPinballs;
};
