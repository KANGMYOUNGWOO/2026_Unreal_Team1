#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PBBossHitPartComponent.generated.h"

class UPrimitiveComponent;

UENUM(BlueprintType)
enum class EPBBossHitPartType : uint8
{
	Body,
	GroggyPoint,
	WeakPoint
};

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossHitPartComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPBBossHitPartComponent();

	UFUNCTION(BlueprintPure, Category = "Boss|Hit")
	EPBBossHitPartType GetHitPartType() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Hit")
	FName GetHitPointName() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Hit")
	void GetHitCollisionComponents(TArray<UPrimitiveComponent*>& OutHitCollisionComponents) const;

	UFUNCTION(BlueprintPure, Category = "Boss|Hit")
	bool IsTargetHitComponent(const UPrimitiveComponent* PrimitiveComponent) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Hit")
	EPBBossHitPartType HitPartType = EPBBossHitPartType::Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Hit")
	FName HitPointName = TEXT("Normal");
};
