#include "PBDirectRewardBumperEffect.h"

#include "EngineUtils.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Reward/PBBumperRewardUtils.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"

namespace
{
	APBCombatPartyController* FindPartyController(const UObject* WorldContext)
	{
		UWorld* World = IsValid(WorldContext) ? WorldContext->GetWorld() : nullptr;
		if (!IsValid(World))
		{
			return nullptr;
		}

		for (TActorIterator<APBCombatPartyController> It(World); It; ++It)
		{
			if (IsValid(*It))
			{
				return *It;
			}
		}
		return nullptr;
	}

	bool CanReceiveResource(AActor* TargetActor, const FName ResourceName)
	{
		UPBBaseResourceComponent* ResourceComponent = IsValid(TargetActor)
			? TargetActor->FindComponentByClass<UPBBaseResourceComponent>()
			: nullptr;
		return IsValid(ResourceComponent)
			&& !ResourceComponent->IsDead()
			&& ResourceComponent->HasResource(ResourceName)
			&& ResourceComponent->GetResourceCurrent(ResourceName)
				< ResourceComponent->GetResourceMax(ResourceName) - KINDA_SMALL_NUMBER;
	}

	float GetResourceRatio(const AActor* TargetActor, const FName ResourceName)
	{
		const UPBBaseResourceComponent* ResourceComponent = IsValid(TargetActor)
			? TargetActor->FindComponentByClass<UPBBaseResourceComponent>()
			: nullptr;
		return IsValid(ResourceComponent)
			? ResourceComponent->GetResourceRatio(ResourceName)
			: 1.0f;
	}

	TArray<AActor*> GetPartyResourceCandidates(
		APBCombatPartyController* PartyController,
		const FName ResourceName,
		AActor* ExcludedActor)
	{
		TArray<AActor*> Candidates;
		if (!IsValid(PartyController))
		{
			return Candidates;
		}

		for (APBBallBase* PartyBall : PartyController->GetValidPartyBalls())
		{
			if (PartyBall != ExcludedActor && CanReceiveResource(PartyBall, ResourceName))
			{
				Candidates.Add(PartyBall);
			}
		}

		Candidates.StableSort([ResourceName](const AActor& Left, const AActor& Right)
		{
			return GetResourceRatio(&Left, ResourceName) < GetResourceRatio(&Right, ResourceName);
		});
		return Candidates;
	}
}

void UPBDirectRewardBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	FPBBumperRewardApplyResult Result;
	if (RewardType != EPBBumperRewardType::Resource
		|| TargetPolicy == EPBDirectRewardTargetPolicy::InteractionActor)
	{
		Result = PBBumperRewardUtils::ApplyReward(
			InteractionActor,
			RewardType,
			ResourceName,
			StatusEffectId,
			EffectData.Power);
	}
	else
	{
		APBCombatPartyController* PartyController = FindPartyController(Bumper);
		if (TargetPolicy == EPBDirectRewardTargetPolicy::LowestResourceRatioPartyMember)
		{
			TArray<AActor*> Candidates = GetPartyResourceCandidates(
				PartyController,
				ResourceName,
				nullptr);
			AActor* RewardTarget = Candidates.IsEmpty() ? InteractionActor : Candidates[0];
			Result = PBBumperRewardUtils::ApplyReward(
				RewardTarget,
				RewardType,
				ResourceName,
				StatusEffectId,
				EffectData.Power);
		}
		else
		{
			float RemainingPower = EffectData.Power;
			const FPBBumperRewardApplyResult InteractionResult =
				PBBumperRewardUtils::ApplyReward(
					InteractionActor,
					RewardType,
					ResourceName,
					StatusEffectId,
					RemainingPower);
			Result.AppliedValue += InteractionResult.AppliedValue;
			Result.AppliedCount += InteractionResult.AppliedCount;
			RemainingPower = FMath::Max(RemainingPower - InteractionResult.AppliedValue, 0.0f);

			for (AActor* Candidate : GetPartyResourceCandidates(
				PartyController,
				ResourceName,
				InteractionActor))
			{
				if (RemainingPower <= KINDA_SMALL_NUMBER)
				{
					break;
				}

				const FPBBumperRewardApplyResult CandidateResult =
					PBBumperRewardUtils::ApplyReward(
						Candidate,
						RewardType,
						ResourceName,
						StatusEffectId,
						RemainingPower);
				Result.AppliedValue += CandidateResult.AppliedValue;
				Result.AppliedCount += CandidateResult.AppliedCount;
				RemainingPower = FMath::Max(
					RemainingPower - CandidateResult.AppliedValue,
					0.0f);
			}
			Result.bApplied = Result.AppliedValue > KINDA_SMALL_NUMBER;
		}
	}

	UE_LOG(LogTemp, Log,
		TEXT("[Bumper] Direct reward resolved. Bumper=%s Target=%s Policy=%s Applied=%s Value=%.2f Count=%d"),
		*GetNameSafe(Bumper),
		*GetNameSafe(InteractionActor),
		*UEnum::GetValueAsString(TargetPolicy),
		Result.bApplied ? TEXT("true") : TEXT("false"),
		Result.AppliedValue,
		Result.AppliedCount);

	FinishEffect();
}
