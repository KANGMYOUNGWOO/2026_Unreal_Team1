#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBTurtleFallingRock.generated.h"

class UProjectileMovementComponent;
class UNiagaraSystem;
class USoundBase;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBTurtleFallingRock : public AActor
{
	GENERATED_BODY()

public:
	APBTurtleFallingRock();

	void SetFallingSpeed(float NewFallingSpeed);
	void SetSourcePatternName(FName NewSourcePatternName);
	void SetHitSFX(USoundBase* NewHitSFX);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleRockHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Turtle|Falling Rock")
	TObjectPtr<UStaticMeshComponent> RockMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Turtle|Falling Rock")
	TObjectPtr<UProjectileMovementComponent> FallingMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Falling Rock", meta = (ClampMin = "0"))
	int32 DamageAmount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Falling Rock", meta = (ClampMin = "0.0"))
	float BounceVelocity = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Falling Rock")
	TObjectPtr<UNiagaraSystem> HitEffect;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> HitSFX;

	FName SourcePatternName = NAME_None;
};
