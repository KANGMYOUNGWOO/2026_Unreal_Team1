#include "PBBattleLaunchRecoveryBumperEffect.h"

#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/GameState/PBBattleGameState.h"

namespace
{
	constexpr int32 MaxLaunchRecoveryPerActivation = 10;
}

void UPBBattleLaunchRecoveryBumperEffect::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	static_cast<void>(InteractionActor);

	UWorld* World = IsValid(Bumper) ? Bumper->GetWorld() : nullptr;
	APBBattleGameState* BattleGameState = IsValid(World)
		? World->GetGameState<APBBattleGameState>()
		: nullptr;
	if (!IsValid(BattleGameState)
		|| !FMath::IsFinite(EffectData.Power)
		|| EffectData.Power <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Battle launch recovery skipped. Bumper=%s GameState=%s Power=%.2f"),
			*GetNameSafe(Bumper),
			*GetNameSafe(BattleGameState),
			EffectData.Power);
		FinishEffect();
		return;
	}

	const int32 RecoveryAmount = FMath::Clamp(
		FMath::RoundToInt(EffectData.Power),
		1,
		MaxLaunchRecoveryPerActivation);
	const int32 PreviousCount = BattleGameState->GetRemainingBattleLaunchCount();
	const int32 SafeMaximum = FMath::Max(MaxRemainingLaunchCount, PreviousCount);
	const int32 NewCount = FMath::Min(PreviousCount + RecoveryAmount, SafeMaximum);
	BattleGameState->SetRemainingBattleLaunchCount(NewCount);

	UE_LOG(LogTemp, Log,
		TEXT("[Bumper] Battle launch count recovered. Bumper=%s Previous=%d Requested=%d New=%d Maximum=%d"),
		*GetNameSafe(Bumper),
		PreviousCount,
		RecoveryAmount,
		NewCount,
		MaxRemainingLaunchCount);
	FinishEffect();
}
