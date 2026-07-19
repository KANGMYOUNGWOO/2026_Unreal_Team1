#include "PBEffectHandler.h"

#include "GameFramework/Actor.h"
#include "PinBallLike/GameState/PBBattleGameState.h"
#include "PinBallLike/Shop/PBShopManager.h"
#include "PinBallLike/Struct/Effect/PBEffectContext.h"
#include "PinBallLike/Struct/Effect/PBEffectTypes.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectParamRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectTableRow.h"

bool UPBEffectHandler::ApplyShopPriceDiscount(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	UPBShopManager* ShopManager = Cast<UPBShopManager>(Context.WorldContextObject.Get());
	if (!ShopManager)
	{
		return false;
	}

	const float Value = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Value));
	FName ModifyType = FindParamName(ParamRows, PBEffectTypes::ParamKey::ModifyType);
	if (ModifyType.IsNone())
	{
		ModifyType = PBEffectTypes::ModifyType::PercentAdd;
	}

	ShopManager->ApplyShopPriceDiscount(ModifyType, Value);
	return !FMath::IsNearlyZero(Value);
}

bool UPBEffectHandler::ApplyShopRerollDiscount(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	UPBShopManager* ShopManager = Cast<UPBShopManager>(Context.WorldContextObject.Get());
	if (!ShopManager)
	{
		return false;
	}

	const float Value = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Value));
	FName ModifyType = FindParamName(ParamRows, PBEffectTypes::ParamKey::ModifyType);
	if (ModifyType.IsNone())
	{
		ModifyType = PBEffectTypes::ModifyType::PercentAdd;
	}

	ShopManager->ApplyShopRerollDiscount(ModifyType, Value);
	return !FMath::IsNearlyZero(Value);
}

bool UPBEffectHandler::ApplySwitchCountBonus(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	APBBattleGameState* BattleGameState = nullptr;
	if (AActor* SourceActor = Context.SourceActor.Get())
	{
		BattleGameState = SourceActor->GetWorld() ? SourceActor->GetWorld()->GetGameState<APBBattleGameState>() : nullptr;
	}
	if (!BattleGameState)
	{
		const UObject* WorldContextObject = Context.WorldContextObject.Get();
		BattleGameState = WorldContextObject && WorldContextObject->GetWorld()
			? WorldContextObject->GetWorld()->GetGameState<APBBattleGameState>()
			: nullptr;
	}
	if (!BattleGameState)
	{
		return false;
	}

	const int32 Count = FMath::Max(0, FCString::Atoi(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Count)));
	if (Count <= 0)
	{
		return false;
	}

	BattleGameState->SetRemainingBattleShiftCount(BattleGameState->GetRemainingBattleShiftCount() + Count);
	return true;
}
