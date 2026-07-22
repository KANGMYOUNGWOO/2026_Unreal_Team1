#include "PinBallLike/Subsystem/PBSoundSubsystem.h"

#include "Components/AudioComponent.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"

void UPBSoundSubsystem::Deinitialize()
{
	if (IsValid(CurrentBGMComponent.Get()))
	{
		CurrentBGMComponent->Stop();
	}

	CurrentBGMComponent = nullptr;
	Super::Deinitialize();
}

UPBSoundSubsystem* UPBSoundSubsystem::Get(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UPBSoundSubsystem>();
}

void UPBSoundSubsystem::PlaySFX(USoundBase* SFXToPlay, float Volume, float Pitch, float StartTime)
{
	UWorld* World = GetWorld();
	if (!IsValid(SFXToPlay) || !World)
	{
		return;
	}

	const float SafeVolume = FMath::IsFinite(Volume) ? FMath::Max(0.0f, Volume) : 1.0f;
	const float SafePitch = FMath::IsFinite(Pitch) ? FMath::Max(0.01f, Pitch) : 1.0f;
	const float SafeStartTime = FMath::IsFinite(StartTime) ? FMath::Max(0.0f, StartTime) : 0.0f;
	UGameplayStatics::PlaySound2D(
		World,
		SFXToPlay,
		SafeVolume * CurrentSFXVolume * MasterVolume,
		SafePitch,
		SafeStartTime);
}

void UPBSoundSubsystem::PlayComboSFX(USoundBase* ComboSFX, float Volume)
{
	UWorld* World = GetWorld();
	if (!IsValid(ComboSFX) || !World)
	{
		return;
	}

	const double CurrentTime = FPlatformTime::Seconds();
	if (LastComboWorld.Get() != World || LastComboPlayTime < 0.0 || CurrentTime - LastComboPlayTime >= ResetCooldown)
	{
		CurrentComboPitch = MinPitch;
	}

	const float SafeVolume = FMath::IsFinite(Volume) ? FMath::Max(0.0f, Volume) : 1.0f;
	UGameplayStatics::PlaySound2D(
		World,
		ComboSFX,
		SafeVolume * CurrentSFXVolume * MasterVolume,
		CurrentComboPitch);

	CurrentComboPitch = FMath::Clamp(CurrentComboPitch + PitchIncrement, MinPitch, MaxPitch);
	LastComboPlayTime = CurrentTime;
	LastComboWorld = World;
}

void UPBSoundSubsystem::PlayBGM(USoundBase* BGMToPlay, float FadeDuration, float Volume)
{
	UWorld* World = GetWorld();
	if (!IsValid(BGMToPlay) || !World)
	{
		return;
	}

	const float SafeFadeDuration = FMath::IsFinite(FadeDuration) ? FMath::Max(0.0f, FadeDuration) : 1.0f;
	const float SafeVolume = FMath::IsFinite(Volume) ? FMath::Max(0.0f, Volume) : 1.0f;

	if (IsValid(CurrentBGMComponent.Get()) && CurrentBGMComponent->IsPlaying()
		&& CurrentBGMComponent->GetSound() == BGMToPlay)
	{
		CurrentBGMVolume = SafeVolume;
		CurrentBGMComponent->SetVolumeMultiplier(CurrentBGMVolume * MasterVolume);
		return;
	}

	UAudioComponent* NewBGMComponent = UGameplayStatics::CreateSound2D(
		World,
		BGMToPlay,
		SafeVolume * MasterVolume,
		1.0f,
		0.0f,
		nullptr,
		true,
		true);

	if (!IsValid(NewBGMComponent))
	{
		return;
	}

	UAudioComponent* PreviousBGMComponent = CurrentBGMComponent.Get();
	CurrentBGMComponent = NewBGMComponent;
	CurrentBGMVolume = SafeVolume;

	if (IsValid(PreviousBGMComponent) && PreviousBGMComponent->IsPlaying())
	{
		PreviousBGMComponent->FadeOut(SafeFadeDuration, 0.0f);
	}

	CurrentBGMComponent->FadeIn(SafeFadeDuration, 1.0f);
}

void UPBSoundSubsystem::StopBGM(float FadeOutDuration)
{
	UAudioComponent* BGMComponentToStop = CurrentBGMComponent.Get();
	CurrentBGMComponent = nullptr;

	if (!IsValid(BGMComponentToStop) || !BGMComponentToStop->IsPlaying())
	{
		return;
	}

	const float SafeFadeOutDuration = FMath::IsFinite(FadeOutDuration)
		? FMath::Max(0.0f, FadeOutDuration)
		: 1.0f;
	BGMComponentToStop->FadeOut(SafeFadeOutDuration, 0.0f);
}

void UPBSoundSubsystem::SetComboSpecs(float InMin, float InMax, float InIncrement, float InCooldown)
{
	MinPitch = FMath::IsFinite(InMin) ? FMath::Max(0.01f, InMin) : 1.0f;
	MaxPitch = FMath::IsFinite(InMax) ? FMath::Max(MinPitch, InMax) : MinPitch;
	PitchIncrement = FMath::IsFinite(InIncrement) ? FMath::Max(0.0f, InIncrement) : 0.0f;
	ResetCooldown = FMath::IsFinite(InCooldown) ? FMath::Max(0.01f, InCooldown) : 1.0f;

	CurrentComboPitch = MinPitch;
	LastComboPlayTime = -1.0;
	LastComboWorld.Reset();
}

void UPBSoundSubsystem::SetMasterVolume(float InVolume)
{
	MasterVolume = FMath::IsFinite(InVolume) ? FMath::Clamp(InVolume, 0.0f, 1.0f) : 1.0f;

	if (IsValid(CurrentBGMComponent.Get()))
	{
		CurrentBGMComponent->SetVolumeMultiplier(CurrentBGMVolume * MasterVolume);
	}
}

void UPBSoundSubsystem::SetSFXVolume(float InVolume)
{
	CurrentSFXVolume = FMath::IsFinite(InVolume)
		? FMath::Clamp(InVolume, 0.0f, 1.0f)
		: 1.0f;
}

void UPBSoundSubsystem::SetBGMVolume(float InVolume)
{
	CurrentBGMVolume = FMath::IsFinite(InVolume)
	? FMath::Clamp(InVolume, 0.0f, 1.0f)
	: 1.0f;

	if (IsValid(CurrentBGMComponent.Get()))
	{
		CurrentBGMComponent->SetVolumeMultiplier(CurrentBGMVolume * MasterVolume);
	}
}
