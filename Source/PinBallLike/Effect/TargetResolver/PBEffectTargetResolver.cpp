#include "PBEffectTargetResolver.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Struct/Effect/PBEffectContext.h"
#include "PinBallLike/Struct/Effect/PBEffectTypes.h"
#include "PinBallLike/Struct/Party/PBPartyTypes.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectTableRow.h"

void UPBEffectTargetResolver::GetSupportedTargetTypes(TArray<FName>& OutTargetTypes) const
{
	OutTargetTypes = {
		PBEffectTypes::TargetType::Ball,
		PBEffectTypes::TargetType::PartyBall,
		PBEffectTypes::TargetType::PartyBalls,
		PBEffectTypes::TargetType::Leader,
		PBEffectTypes::TargetType::Follower,
		PBEffectTypes::TargetType::Followers,
		PBEffectTypes::TargetType::Battle,
		PBEffectTypes::TargetType::Shop
	};
}

void UPBEffectTargetResolver::ResolveTargetActors(
	const FPBEffectTableRow& EffectRow,
	const FPBEffectContext& Context,
	TArray<TObjectPtr<AActor>>& OutTargetActors) const
{
	OutTargetActors.Reset();
	AddMatchingContextActors(EffectRow, Context, OutTargetActors);
}

void UPBEffectTargetResolver::AddMatchingContextActors(
	const FPBEffectTableRow& EffectRow,
	const FPBEffectContext& Context,
	TArray<TObjectPtr<AActor>>& OutTargetActors) const
{
	for (AActor* TargetActor : Context.TargetActors)
	{
		if (!IsValid(TargetActor))
		{
			continue;
		}

		if (!DoesActorMatchTargetType(TargetActor, EffectRow.TargetType))
		{
			continue;
		}

		if (!DoesActorMatchTargetFilter(TargetActor, EffectRow.TargetFilter))
		{
			continue;
		}

		OutTargetActors.Add(TargetActor);
	}

	if (OutTargetActors.IsEmpty()
		&& IsValid(Context.SourceActor)
		&& DoesActorMatchTargetType(Context.SourceActor, EffectRow.TargetType)
		&& DoesActorMatchTargetFilter(Context.SourceActor, EffectRow.TargetFilter))
	{
		OutTargetActors.Add(Context.SourceActor);
	}
}

bool UPBEffectTargetResolver::DoesActorMatchTargetType(const AActor* Actor, const FName TargetType) const
{
	if (!IsValid(Actor))
	{
		return false;
	}

	if (TargetType == PBEffectTypes::TargetType::Ball
		|| TargetType == PBEffectTypes::TargetType::PartyBall
		|| TargetType == PBEffectTypes::TargetType::PartyBalls
		|| TargetType == PBEffectTypes::TargetType::Leader
		|| TargetType == PBEffectTypes::TargetType::Follower
		|| TargetType == PBEffectTypes::TargetType::Followers)
	{
		const APBBallBase* Ball = Cast<APBBallBase>(Actor);
		if (!IsValid(Ball))
		{
			return false;
		}

		if (TargetType == PBEffectTypes::TargetType::Leader)
		{
			return Ball->GetCombatRole() == EPBBallPartyRole::Leader;
		}

		if (TargetType == PBEffectTypes::TargetType::Follower
			|| TargetType == PBEffectTypes::TargetType::Followers)
		{
			return Ball->GetCombatRole() == EPBBallPartyRole::Follower;
		}

		return true;
	}

	return TargetType == PBEffectTypes::TargetType::Battle
		|| TargetType == PBEffectTypes::TargetType::Shop;
}

bool UPBEffectTargetResolver::DoesActorMatchTargetFilter(const AActor* Actor, const FName TargetFilter) const
{
	if (!IsValid(Actor) || TargetFilter.IsNone() || TargetFilter == PBEffectTypes::TargetFilter::All)
	{
		return IsValid(Actor);
	}

	const APBBallBase* Ball = Cast<APBBallBase>(Actor);
	if (IsValid(Ball) && Ball->GetBallId() == TargetFilter)
	{
		return true;
	}

	return Actor->ActorHasTag(TargetFilter);
}
