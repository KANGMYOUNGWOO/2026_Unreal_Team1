#include "PBBumperSoundComponent.h"

#include "PinBallLike/Subsystem/PBSoundSubsystem.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

UPBBumperSoundComponent::UPBBumperSoundComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<USoundBase> TriggerCountSoundAsset(
		TEXT("/Game/Resources/Sound/SFX/Battle/S_Bound.S_Bound"));
	if (TriggerCountSoundAsset.Succeeded())
	{
		TriggerCountSound = TriggerCountSoundAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> ActivationSoundAsset(
		TEXT("/Game/Resources/Sound/SFX/Battle/S_Notice02.S_Notice02"));
	if (ActivationSoundAsset.Succeeded())
	{
		ActivationSound = ActivationSoundAsset.Object;
	}
}

void UPBBumperSoundComponent::PlayTriggerCountSound(
	const int32 CurrentCount,
	const int32 RequiredCount)
{
	UPBSoundSubsystem* SoundSubsystem = UPBSoundSubsystem::Get(this);
	if (!IsValid(TriggerCountSound) || !SoundSubsystem || RequiredCount <= 0)
	{
		return;
	}

	const float Progress = RequiredCount <= 1
		? 1.0f
		: FMath::Clamp(
			static_cast<float>(CurrentCount - 1) / static_cast<float>(RequiredCount - 1),
			0.0f,
			1.0f);
	const float SafeMinimumPitch = FMath::IsFinite(MinimumTriggerPitch)
		? FMath::Max(MinimumTriggerPitch, 0.01f)
		: 1.0f;
	const float SafeMaximumPitch = FMath::IsFinite(MaximumTriggerPitch)
		? FMath::Max(MaximumTriggerPitch, SafeMinimumPitch)
		: SafeMinimumPitch;
	const float Pitch = FMath::Lerp(SafeMinimumPitch, SafeMaximumPitch, Progress);

	SoundSubsystem->PlaySFX(TriggerCountSound, TriggerSoundVolume, Pitch);
}

void UPBBumperSoundComponent::PlayActivationSound()
{
	UPBSoundSubsystem* SoundSubsystem = UPBSoundSubsystem::Get(this);
	if (!IsValid(ActivationSound) || !SoundSubsystem)
	{
		return;
	}

	SoundSubsystem->PlaySFX(ActivationSound, ActivationSoundVolume);
}
