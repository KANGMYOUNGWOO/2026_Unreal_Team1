#include "PBRelicTriggerService.h"

#include "PinBallLike/Subsystem/Relic/PBRelicSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"

void UPBRelicTriggerService::Initialize(
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
			TEXT("[RelicTriggerService] RelicSubsystem is invalid."));
	}

	if (!IsValid(TableSubsystem))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[RelicTriggerService] TableSubsystem is invalid."));
	}
}

void UPBRelicTriggerService::NotifyRelicEvent(
	const FName TriggerEvent,
	const FPBRelicTriggerContext& Context)
{
	if (TriggerEvent.IsNone())
	{
		return;
	}

	if (!IsValid(RelicSubsystem) ||
		!IsValid(TableSubsystem))
	{
		return;
	}

	for (const FName RelicId :
		RelicSubsystem->GetOwnedRelicIds())
	{
		HandleRelicTrigger(
			RelicId,
			TriggerEvent,
			Context);
	}
}

void UPBRelicTriggerService::HandleRelicTrigger(
	const FName RelicId,
	const FName TriggerEvent,
	const FPBRelicTriggerContext& Context)
{
	if (RelicId.IsNone())
	{
		return;
	}

	/*
	 * 현재는 MVP 하드코딩.
	 *
	 * 나중에는 DT_RelicTrigger를 만들어서
	 * 데이터 기반으로 변경한다.
	 */

	if (RelicId == TEXT("Relic_ComboAttack") &&
		TriggerEvent == TEXT("ComboReached"))
	{
		if (Context.IntValue < 100)
		{
			return;
		}

		UPBStatusEffectComponent* StatusEffectComponent =
			FindStatusEffectComponent(Context.SourceActor);

		if (!IsValid(StatusEffectComponent))
		{
			return;
		}

		StatusEffectComponent->ApplyStatusEffect(
			TEXT("Status_Relic_ComboAttack"));

		UE_LOG(
			LogTemp,
			Log,
			TEXT("[RelicTriggerService] Relic triggered. RelicId=%s Event=%s"),
			*RelicId.ToString(),
			*TriggerEvent.ToString());

		return;
	}

	if (RelicId == TEXT("Relic_BattleShield") &&
		TriggerEvent == TEXT("BattleStarted"))
	{
		UPBStatusEffectComponent* StatusEffectComponent =
			FindStatusEffectComponent(Context.SourceActor);

		if (!IsValid(StatusEffectComponent))
		{
			return;
		}

		StatusEffectComponent->ApplyStatusEffect(
			TEXT("Status_Relic_BattleShield"));

		UE_LOG(
			LogTemp,
			Log,
			TEXT("[RelicTriggerService] Relic triggered. RelicId=%s Event=%s"),
			*RelicId.ToString(),
			*TriggerEvent.ToString());

		return;
	}
}

UPBStatusEffectComponent*
UPBRelicTriggerService::FindStatusEffectComponent(
	AActor* TargetActor) const
{
	if (!IsValid(TargetActor))
	{
		return nullptr;
	}

	return TargetActor->FindComponentByClass<
		UPBStatusEffectComponent>();
}