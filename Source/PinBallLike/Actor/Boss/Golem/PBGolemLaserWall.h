#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBGolemLaserWall.generated.h"

class UBoxComponent;
class UPrimitiveComponent;

UCLASS()
class PINBALLLIKE_API APBGolemLaserWall : public AActor
{
	GENERATED_BODY()

public:
	APBGolemLaserWall();

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem|Laser")
	void InitializeLaserWall(FVector InLaunchDirection, float InBounceVelocity, FName InSourcePatternName);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem|Laser")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem|Laser")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem|Laser", meta = (ClampMin = "0.0", Units = "cm/s"))
	float BounceVelocity = 1500.0f;

private:
	void BounceBall(AActor* OtherActor, const FHitResult& Hit);

	UFUNCTION()
	void HandleCollisionHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	FVector LaunchDirection = FVector::ForwardVector;
	FName SourcePatternName = NAME_None;
};
