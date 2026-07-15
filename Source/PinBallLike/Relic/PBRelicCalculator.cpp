#include "PBRelicCalculator.h"

#include "PinBallLike/Subsystem/Relic/PBRelicSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Relic/Struct/PBRelicModifierRow.h"

void UPBRelicCalculator::Initialize(
	UPBRelicSubsystem* InRelicSubsystem,
	UPBTableDataSubsystem* InTableSubsystem)
{
	RelicSubsystem = InRelicSubsystem;
	TableSubsystem = InTableSubsystem;

	if (!IsValid(RelicSubsystem))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[RelicCalculator] RelicSubsystem is invalid."));
	}

	if (!IsValid(TableSubsystem))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[RelicCalculator] TableSubsystem is invalid."));
	}
}

int32 UPBRelicCalculator::CalculateBallStat(
	const FName StatName,
	const int32 BaseValue) const
{
	const float FinalValue = CalculateModifiedValue(
		EPBRelicTargetType::Ball,
		StatName,
		static_cast<float>(BaseValue));

	return FMath::RoundToInt(FinalValue);
}

float UPBRelicCalculator::CalculateBallStat(
	const FName StatName,
	const float BaseValue) const
{
	return CalculateModifiedValue(
		EPBRelicTargetType::Ball,
		StatName,
		BaseValue);
}

float UPBRelicCalculator::CalculateShopPrice(
	const float BasePrice) const
{
	return CalculateModifiedValue(
		EPBRelicTargetType::Shop,
		TEXT("Price"),
		BasePrice);
}

float UPBRelicCalculator::CalculateModifiedValue(
	const EPBRelicTargetType TargetType,
	const FName TargetStat,
	const float BaseValue) const
{
	if (!IsValid(RelicSubsystem) ||
		!IsValid(TableSubsystem))
	{
		return BaseValue;
	}

	float AddValue = 0.0f;
	float MultiplyValue = 1.0f;

	bool bHasSetValue = false;
	float SetValue = 0.0f;

	for (const FName RelicId : RelicSubsystem->GetOwnedRelicIds())
	{
		TArray<FPBRelicModifierRow> ModifierRows;

		TableSubsystem->GetRelicModifierRows(
			RelicId,
			ModifierRows);

		for (const FPBRelicModifierRow& Modifier : ModifierRows)
		{
			if (Modifier.TargetType != TargetType)
			{
				continue;
			}

			if (Modifier.TargetStat != TargetStat)
			{
				continue;
			}

			switch (Modifier.ModifyType)
			{
			case EPBRelicModifyType::Add:
				AddValue += Modifier.Value;
				break;

			case EPBRelicModifyType::Multiply:
				MultiplyValue += Modifier.Value;
				break;

			case EPBRelicModifyType::Set:
				bHasSetValue = true;
				SetValue = Modifier.Value;
				break;

			default:
				break;
			}
		}
	}

	float FinalValue =
		(BaseValue + AddValue) * MultiplyValue;

	if (bHasSetValue)
	{
		FinalValue = SetValue;
	}

	const float MinimumValue =
		FMath::Max(
			BaseValue * 0.1f,
			1.0f);

	return FMath::Max(
		FinalValue,
		MinimumValue);
}