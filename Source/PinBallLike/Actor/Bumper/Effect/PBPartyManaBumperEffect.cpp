#include "PBPartyManaBumperEffect.h"

#include "EngineUtils.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"

namespace
{
	constexpr float MaxManaRecoveryPerActivation = 10000.0f;
}

void UPBPartyManaBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	static_cast<void>(InteractionActor);

	if (!IsValid(Bumper) || !FMath::IsFinite(EffectData.Power) || EffectData.Power <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Party mana recovery skipped. Bumper=%s Power=%.2f"),
			*GetNameSafe(Bumper),
			EffectData.Power);
		FinishEffect();
		return;
	}

	APBCombatPartyController* PartyController = nullptr;
	for (TActorIterator<APBCombatPartyController> It(Bumper->GetWorld()); It; ++It)
	{
		if (IsValid(*It))
		{
			PartyController = *It;
			break;
		}
	}

	if (!IsValid(PartyController))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Party mana recovery skipped because no party controller was found. Bumper=%s"),
			*GetNameSafe(Bumper));
		FinishEffect();
		return;
	}

	const float RecoveryAmount = FMath::Min(EffectData.Power, MaxManaRecoveryPerActivation);
	int32 AffectedBallCount = 0;
	float TotalRecoveredMana = 0.0f;
	for (APBBallBase* Ball : PartyController->GetValidPartyBalls())
	{
		UPBBaseResourceComponent* ResourceComponent = IsValid(Ball)
			? Ball->GetResourceComponent()
			: nullptr;
		if (!IsValid(ResourceComponent)
			|| ResourceComponent->IsDead()
			|| !ResourceComponent->HasResource(PBResourceNames::Mana))
		{
			continue;
		}

		const float PreviousMana = ResourceComponent->GetResourceCurrent(PBResourceNames::Mana);
		ResourceComponent->ApplyResourceDelta(PBResourceNames::Mana, RecoveryAmount);
		const float RecoveredMana =
			ResourceComponent->GetResourceCurrent(PBResourceNames::Mana) - PreviousMana;
		if (RecoveredMana > 0.0f)
		{
			++AffectedBallCount;
			TotalRecoveredMana += RecoveredMana;
		}
	}

	UE_LOG(LogTemp, Log,
		TEXT("[Bumper] Party mana recovery completed. Bumper=%s RequestedPerBall=%.2f AffectedBalls=%d TotalRecovered=%.2f"),
		*GetNameSafe(Bumper),
		RecoveryAmount,
		AffectedBallCount,
		TotalRecoveredMana);

	FinishEffect();
}
