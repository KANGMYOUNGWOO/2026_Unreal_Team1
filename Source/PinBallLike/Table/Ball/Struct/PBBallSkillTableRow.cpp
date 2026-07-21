#include "PinBallLike/Table/Ball/Struct/PBBallSkillTableRow.h"

int32 FPBBallSkillTableRow::CalculateBaseDamage(const int32 BallAttackPower) const
{
	if (PowerValue <= 0.0f)
	{
		return 0;
	}

	const int32 CalculatedDamage =
		FMath::RoundToInt(static_cast<float>(BallAttackPower) * PowerValue);
	return CalculatedDamage < 0 ? 0 : FMath::Max(CalculatedDamage, 1);
}

FText FPBBallSkillTableRow::GetDescription(const int32 BallAttackPower) const
{
	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("Power"), FText::AsNumber(CalculateBaseDamage(BallAttackPower)));
	Arguments.Add(TEXT("Life"), FText::AsNumber(LifeValue));
	Arguments.Add(TEXT("Effect"), FText::AsNumber(EffectValue));
	Arguments.Add(TEXT("Groggy"), FText::AsNumber(GroggyValue));

	return FText::Format(Description, Arguments);
}
