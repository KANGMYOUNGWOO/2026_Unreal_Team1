#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBossWeaknessComponent.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FBossWeaknessData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Weakness", meta = (ClampMin = "0"))
	int32 HPDamageMultiplierPercent = 200;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBBossWeaknessOpenChangedSignature, bool, IsWeaknessOpen);

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossWeaknessComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBossWeaknessComponent();

	UFUNCTION(BlueprintCallable, Category = "Boss|Weakness")
	void OpenWeakness();

	UFUNCTION(BlueprintCallable, Category = "Boss|Weakness")
	void CloseWeakness();

	UFUNCTION(BlueprintPure, Category = "Boss|Weakness")
	bool IsWeaknessPoint(FName WeaknessPointName) const;

	UFUNCTION(BlueprintPure, Category = "Boss|Weakness")
	bool IsWeaknessPointOpen(FName WeaknessPointName) const;

	UFUNCTION(BlueprintPure, Category = "Boss|Weakness")
	int32 CalculateWeaknessDamage(FName WeaknessPointName, int32 DamageAmount) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Weakness")
	bool IsWeaknessOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Weakness")
	TMap<FName, FBossWeaknessData> WeaknessPointDataMap;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Weakness")
	FPBBossWeaknessOpenChangedSignature OnWeaknessOpenChanged;
};
