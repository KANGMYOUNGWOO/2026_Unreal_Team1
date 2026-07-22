#include "PBSkillFeedbackComponent.h"

#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Subsystem/PBSoundSubsystem.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

UPBSkillFeedbackComponent::UPBSkillFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<USoundBase> CastSoundAsset(
		TEXT("/Game/Resources/Sound/SFX/Battle/S_SkillReady.S_SkillReady"));
	if (CastSoundAsset.Succeeded())
	{
		CastFeedback.Sound = CastSoundAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> HitSoundAsset(
		TEXT("/Game/Resources/Sound/SFX/Battle/S_Hit.S_Hit"));
	if (HitSoundAsset.Succeeded())
	{
		HitFeedback.Sound = HitSoundAsset.Object;
	}

	HitFeedback.MinimumPitch = 0.95f;
	HitFeedback.MaximumPitch = 1.05f;
}

void UPBSkillFeedbackComponent::PlayCastFeedback()
{
	PlayFeedback(CastFeedback);
}

void UPBSkillFeedbackComponent::PlayHitFeedback()
{
	PlayFeedback(HitFeedback);
}

void UPBSkillFeedbackComponent::PlaySkillSound(
	USoundBase* Sound,
	const float Volume,
	const float Pitch)
{
	UPBSoundSubsystem* SoundSubsystem = UPBSoundSubsystem::Get(this);
	UWorld* World = GetWorld();
	if (!IsValid(Sound) || !SoundSubsystem || !World)
	{
		return;
	}

	StopSkillSound();

	const float SafeVolume = FMath::IsFinite(Volume) ? FMath::Max(Volume, 0.0f) : 1.0f;
	const float SafePitch = FMath::IsFinite(Pitch) ? FMath::Max(Pitch, 0.01f) : 1.0f;
	ActiveSkillAudioComponent = UGameplayStatics::CreateSound2D(
		World,
		Sound,
		SafeVolume * SoundSubsystem->GetSFXVolume() * SoundSubsystem->GetMasterVolume(),
		SafePitch,
		0.0f,
		nullptr,
		false,
		true);

	if (IsValid(ActiveSkillAudioComponent))
	{
		ActiveSkillAudioComponent->Play();
	}
}

void UPBSkillFeedbackComponent::StopSkillSound()
{
	UAudioComponent* AudioComponentToStop = ActiveSkillAudioComponent;
	ActiveSkillAudioComponent = nullptr;
	if (!IsValid(AudioComponentToStop) || !AudioComponentToStop->IsPlaying())
	{
		return;
	}

	const float SafeFadeOutDuration = FMath::IsFinite(SkillSoundFadeOutDuration)
		? FMath::Max(SkillSoundFadeOutDuration, 0.0f)
		: 0.2f;
	if (SafeFadeOutDuration <= 0.0f)
	{
		AudioComponentToStop->Stop();
		return;
	}

	AudioComponentToStop->FadeOut(SafeFadeOutDuration, 0.0f);
}

bool UPBSkillFeedbackComponent::PlayCameraShake(
	TSubclassOf<UCameraShakeBase> CameraShakeClass,
	const float Scale,
	const float MinimumInterval)
{
	UWorld* World = GetWorld();
	const float SafeScale = FMath::IsFinite(Scale) ? FMath::Max(Scale, 0.0f) : 1.0f;
	if (!CameraShakeClass || !World || SafeScale <= 0.0f)
	{
		return false;
	}

	const double CurrentTime = World->GetTimeSeconds();
	const double SafeMinimumInterval = FMath::IsFinite(MinimumInterval)
		? FMath::Max(MinimumInterval, 0.0f)
		: 0.0f;
	if (LastCameraShakeTime >= 0.0
		&& CurrentTime - LastCameraShakeTime < SafeMinimumInterval)
	{
		return false;
	}

	bool bStartedAnyShake = false;
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (!IsValid(PlayerController))
		{
			continue;
		}

		PlayerController->ClientStartCameraShake(CameraShakeClass, SafeScale);
		bStartedAnyShake = true;
	}

	if (bStartedAnyShake)
	{
		LastCameraShakeTime = CurrentTime;
	}
	return bStartedAnyShake;
}

void UPBSkillFeedbackComponent::PlayFeedback(
	const FPBSkillFeedbackSettings& FeedbackSettings)
{
	if (UPBSoundSubsystem* SoundSubsystem = UPBSoundSubsystem::Get(this))
	{
		SoundSubsystem->PlaySFX(FeedbackSettings.Sound, 1.0f, 1.0f);
	}

	//TODO 약간 확대?
	
	//PlayCameraShake(
	//	FeedbackSettings.CameraShakeClass,
	//	FeedbackSettings.CameraShakeScale,
	//	FeedbackSettings.MinimumCameraShakeInterval);
}

void UPBSkillFeedbackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSkillSound();
	Super::EndPlay(EndPlayReason);
}
