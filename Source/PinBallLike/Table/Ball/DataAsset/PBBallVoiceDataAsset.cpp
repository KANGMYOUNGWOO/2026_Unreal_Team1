#include "PBBallVoiceDataAsset.h"

#include "Sound/SoundBase.h"

USoundBase* UPBBallVoiceDataAsset::GetRandomDeathSound(const EPBBallVoiceType VoiceType) const
{
	const FPBBallVoiceSoundList* SoundList = VoiceSounds.Find(VoiceType);
	if (!SoundList || SoundList->DeathSounds.IsEmpty())
	{
		return nullptr;
	}

	const int32 StartIndex = FMath::RandHelper(SoundList->DeathSounds.Num());
	for (int32 Offset = 0; Offset < SoundList->DeathSounds.Num(); ++Offset)
	{
		if (USoundBase* Sound = SoundList->DeathSounds[(StartIndex + Offset) % SoundList->DeathSounds.Num()])
		{
			return Sound;
		}
	}

	return nullptr;
}
