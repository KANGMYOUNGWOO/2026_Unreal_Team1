#include "PinBallLike/Table/Ball/Struct/PBBallSkillTableRow.h"

int32 FPBBallSkillTableRow::CalculateBaseDamage(const int32 BallAttackPower) const
{
	return FMath::Max(
		0,
		FMath::RoundToInt(static_cast<float>(BallAttackPower) * PowerValue));
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
