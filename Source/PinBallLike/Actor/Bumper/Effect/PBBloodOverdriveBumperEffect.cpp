#include "PBBloodOverdriveBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Component/PBBumperReactiveRepairComponent.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperSharedEffectAdapter.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Reward/PBBumperRewardUtils.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"

void UPBBloodOverdriveBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	static const TArray<FName> RequiredParameters =
	{
		TEXT("ResourceName"),
		TEXT("ResourceCost"),
		TEXT("StatName"),
		TEXT("ModifyType"),
		TEXT("Value"),
		TEXT("Duration"),
		TEXT("MinRemainingResource")
	};

	FPBBumperSharedEffectDefinition Definition;
	FString ResolveError;
	FName SheetResourceName;
	FName RuntimeResourceName;
	FName SheetStatName;
	FName RuntimeStatName;
	FName ModifyType;
	float ResourceCost = 0.0f;
	float StatPercent = 0.0f;
	float Duration = 0.0f;
	float MinRemainingResource = 0.0f;
	const bool bResolved = PBBumperSharedEffectAdapter::Resolve(
		Bumper,
		EffectData.SharedEffectId,
		TEXT("ResourceCostStatBuff"),
		RequiredParameters,
		Definition,
		ResolveError)
		&& Definition.TryGetName(TEXT("ResourceName"), SheetResourceName)
		&& PBBumperSharedEffectAdapter::ResolveResourceName(SheetResourceName, RuntimeResourceName)
		&& Definition.TryGetFloat(TEXT("ResourceCost"), ResourceCost)
		&& Definition.TryGetName(TEXT("StatName"), SheetStatName)
		&& PBBumperSharedEffectAdapter::ResolveStatName(SheetStatName, RuntimeStatName)
		&& Definition.TryGetName(TEXT("ModifyType"), ModifyType)
		&& ModifyType == TEXT("PercentAdd")
		&& Definition.TryGetFloat(TEXT("Value"), StatPercent)
		&& Definition.TryGetFloat(TEXT("Duration"), Duration)
		&& Definition.TryGetFloat(TEXT("MinRemainingResource"), MinRemainingResource)
		&& ResourceCost > 0.0f
		&& StatPercent > 0.0f
		&& Duration > 0.0f
		&& MinRemainingResource >= 0.0f;

	UPBBaseResourceComponent* ResourceComponent = IsValid(InteractionActor)
		? InteractionActor->FindComponentByClass<UPBBaseResourceComponent>()
		: nullptr;
	if (!bResolved
		|| !IsValid(ResourceComponent)
		|| ResourceComponent->IsDead()
		|| !ResourceComponent->HasResource(RuntimeResourceName)
		|| ResourceComponent->GetResourceCurrent(RuntimeResourceName) - ResourceCost
			< MinRemainingResource - KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogTemp, Log,
			TEXT("[Bumper] Blood overdrive skipped. Bumper=%s Target=%s Error=%s Current=%.2f Cost=%.2f Minimum=%.2f"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			*ResolveError,
			IsValid(ResourceComponent)
				? ResourceComponent->GetResourceCurrent(RuntimeResourceName)
				: 0.0f,
			ResourceCost,
			MinRemainingResource);
		FinishEffect();
		return;
	}

	// 반응형 수리는 피격 후 회복 효과이므로, 이 효과가 스스로 지불하는 HP는 피격 횟수로 세지 않는다.
	if (UPBBumperReactiveRepairComponent* RepairComponent =
		InteractionActor->FindComponentByClass<UPBBumperReactiveRepairComponent>())
	{
		RepairComponent->SuppressNextIntentionalDecrease(RuntimeResourceName);
	}
	ResourceComponent->ApplyResourceDelta(RuntimeResourceName, -ResourceCost);
	const FPBBumperRewardApplyResult Result = PBBumperRewardUtils::ApplyReward(
		InteractionActor,
		EPBBumperRewardType::TimedStatPercent,
		NAME_None,
		NAME_None,
		StatPercent,
		EffectSourceId,
		Duration,
		RuntimeStatName);
	if (!Result.bApplied)
	{
		ResourceComponent->ApplyResourceDelta(RuntimeResourceName, ResourceCost);
	}

	if (Result.bApplied)
	{
		UE_LOG(LogTemp, Log,
			TEXT("[Bumper] Blood overdrive resolved. Bumper=%s Target=%s Cost=%.2f Stat=%s Percent=%.1f Duration=%.1f"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor),
			ResourceCost,
			*RuntimeStatName.ToString(),
			StatPercent,
			Duration);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Blood overdrive failed and refunded its resource cost. Bumper=%s Target=%s"),
			*GetNameSafe(Bumper),
			*GetNameSafe(InteractionActor));
	}
	FinishEffect();
}
