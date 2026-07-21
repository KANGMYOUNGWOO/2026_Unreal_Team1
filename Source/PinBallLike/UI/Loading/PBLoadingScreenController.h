#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"

class SLoadingScreenLayout;
class UWorld;

class FPBLoadingScreenController
{
public:
	void Show(UWorld* World);
	void Hide();
	void Shutdown();

private:
	enum class EFadeState : uint8
	{
		Hidden,
		Visible,
		FadingOut
	};

	static constexpr float FadeDurationSeconds = 0.75f;
	static constexpr float FadeUpdateIntervalSeconds = 0.016f;

	bool EnsureLoadingScreen(UWorld* World);
	void StartFadeOut();
	void UpdateFadeOut();
	void Remove();
	void ClearFadeTimer();

	TWeakObjectPtr<UWorld> LoadingWorld;
	TSharedPtr<SLoadingScreenLayout> LoadingScreenWidget;
	FTimerHandle FadeTimerHandle;
	float FadeAlpha = 0.0f;
	EFadeState FadeState = EFadeState::Hidden;
};
