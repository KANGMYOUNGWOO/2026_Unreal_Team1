#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PinBallLike/Struct/Relic/PBRelicTypes.h"
#include "PBRelicCalculator.generated.h"

class UPBRelicSubsystem;
class UPBTableDataSubsystem;

UCLASS()
class PINBALLLIKE_API UPBRelicCalculator : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(
		UPBRelicSubsystem* InRelicSubsystem,
		UPBTableDataSubsystem* InTableSubsystem);

	int32 CalculateBallStat(
		FName StatName,
		int32 BaseValue) const;

	float CalculateBallStat(
		FName StatName,
		float BaseValue) const;

	float CalculateShopPrice(
		float BasePrice) const;

private:
	float CalculateModifiedValue(
		EPBRelicTargetType TargetType,
		FName TargetStat,
		float BaseValue) const;

private:
	UPROPERTY()
	TObjectPtr<UPBRelicSubsystem> RelicSubsystem;

	UPROPERTY()
	TObjectPtr<UPBTableDataSubsystem> TableSubsystem;
};