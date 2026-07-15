#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBossHitEffectComponent.generated.h"

class UMaterialInstanceDynamic;

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

private:
	void InitializeMaterials();
	void ApplyColor(float Alpha);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

	TArray<FLinearColor> OriginalColors;
	float ElapsedTime = 0.0f;
};
