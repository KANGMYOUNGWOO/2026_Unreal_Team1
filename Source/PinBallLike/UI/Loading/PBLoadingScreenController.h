#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"

class SLoadingScreenLayout;
class UWorld;

class FPBLoadingScreenController
{
public:
	void Show(UWorld* World, bool IsFadeIn);
	void Hide();
	void Shutdown();

private:
	static constexpr float FadeDurationSeconds = 0.75f;
	static constexpr float FadeUpdateIntervalSeconds = 0.016f;

	void UpdateFadeIn();
	void UpdateFadeOut();
	void Remove();
	void ClearFadeTimer();

	TWeakObjectPtr<UWorld> LoadingWorld;
	TSharedPtr<SLoadingScreenLayout> LoadingScreenWidget;
	FTimerHandle FadeTimerHandle;
	float FadeAlpha = 0.0f;
};
