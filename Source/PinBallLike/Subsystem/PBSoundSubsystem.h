// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBSoundSubsystem.generated.h"

class USoundBase;
class UAudioComponent;

UCLASS(BlueprintType)
class PINBALLLIKE_API UPBSoundSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Sound|Subsystem", meta = (WorldContext = "WorldContextObject"))
	static UPBSoundSubsystem* Get(const UObject* WorldContextObject);

	// 월드 어디서나 2D 사운드를 즉시 재생합니다.
	UFUNCTION(BlueprintCallable, Category = "Sound|SFX")
	void PlaySFX(USoundBase* SFXToPlay, float Volume = 1.0f, float Pitch = 1.0f, float StartTime = 0.0f);

	// 쿨다운 안에 반복 호출하면 피치가 단계적으로 올라갑니다.
	UFUNCTION(BlueprintCallable, Category = "Sound|SFX")
	void PlayComboSFX(USoundBase* ComboSFX, float Volume = 1.0f);

	// 기존 BGM과 새 BGM을 크로스페이드합니다. 반복 재생은 사운드 에셋에서 설정해야 합니다.
	UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
	void PlayBGM(USoundBase* BGMToPlay, float FadeDuration = 1.0f, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
	void StopBGM(float FadeOutDuration = 1.0f);

	// 콤보 설정을 갱신하고 현재 콤보를 초기화합니다.
	UFUNCTION(BlueprintCallable, Category = "Sound|Combo")
	void SetComboSpecs(float InMin, float InMax, float InIncrement, float InCooldown);

	UFUNCTION(BlueprintPure, Category = "Sound|Volume")
	float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintCallable, Category = "Sound|Volume")
	void SetMasterVolume(float InVolume);

	UFUNCTION(BlueprintPure, Category = "Sound|Volume")
	float GetSFXVolume() const { return CurrentSFXVolume; }

	UFUNCTION(BlueprintCallable, Category = "Sound|Volume")
	void SetSFXVolume(float InVolume);

	UFUNCTION(BlueprintPure, Category = "Sound|Volume")
	float GetBGMVolume() const { return CurrentBGMVolume; }

	UFUNCTION(BlueprintCallable, Category = "Sound|Volume")
	void SetBGMVolume(float InVolume);
	

private:
	float MinPitch = 1.0f;
	float MaxPitch = 4.0f;
	float PitchIncrement = 0.25f;
	float ResetCooldown = 1.0f;

	float CurrentComboPitch = 1.0f;
	double LastComboPlayTime = -1.0;
	TWeakObjectPtr<UWorld> LastComboWorld;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> CurrentBGMComponent;

	float CurrentBGMVolume = 1.0f;
	float CurrentSFXVolume = 1.0f;
	float MasterVolume = 0.5f;
};
