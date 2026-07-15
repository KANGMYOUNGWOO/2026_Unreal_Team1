#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBossHitEffectComponent.generated.h"

class UMaterialInstanceDynamic;
class UPrimitiveComponent;

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossHitEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBossHitEffectComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Hit Effect")
	void PlayHitEffect();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Hit Effect")
	FName ColorParameterName = TEXT("HitColor");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Hit Effect")
	FLinearColor HitColor = FLinearColor(1.0f, 0.35f, 0.35f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Hit Effect", meta = (ClampMin = "0.01"))
	float HitFlashDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Hit Effect", meta = (ClampMin = "0"))
	float PinballHitVelocity = 500.0f;

private:
	UFUNCTION()
	void HandleOwnerComponentHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	void InitializeMaterials();
	void BindOwnerCollisionEvents();
	void ApplyPinballHitVelocity(AActor* OtherActor, const FHitResult& Hit) const;
	void ApplyColor(float Alpha);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

	TArray<FLinearColor> OriginalColors;
	float ElapsedTime = 0.0f;
};
