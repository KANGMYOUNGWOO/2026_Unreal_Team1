#include "PBLoadingScreenController.h"

#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "PinBallLike/UI/Loading/PBLoadingScreen.h"
#include "SLoadingScreenLayout.h"

void FPBLoadingScreenController::Show(UWorld* World, const bool IsFadeIn)
{
	if (LoadingScreenWidget.IsValid() || !World || !World->GetGameViewport())
	{
		return;
	}

	LoadingWorld = World;
	LoadingScreenWidget = FPBLoadingScreen::CreateLoadingScreenWidget();
	FadeAlpha = IsFadeIn ? 0.0f : 1.0f;
	LoadingScreenWidget->SetLoadingContentOpacity(1.0f);
	LoadingScreenWidget->SetRenderOpacity(FadeAlpha);
	World->GetGameViewport()->AddViewportWidgetContent(LoadingScreenWidget.ToSharedRef(), MAX_int32);

	if (IsFadeIn)
	{
		World->GetTimerManager().SetTimer(
			FadeTimerHandle,
			FTimerDelegate::CreateRaw(this, &FPBLoadingScreenController::UpdateFadeIn),
			FadeUpdateIntervalSeconds,
			true);
	}
}

void FPBLoadingScreenController::Hide()
{
	if (!LoadingScreenWidget.IsValid() || !LoadingWorld.IsValid())
	{
		return;
	}

	ClearFadeTimer();
	LoadingWorld->GetTimerManager().SetTimer(
		FadeTimerHandle,
		FTimerDelegate::CreateRaw(this, &FPBLoadingScreenController::UpdateFadeOut),
		FadeUpdateIntervalSeconds,
		true);
}

void FPBLoadingScreenController::Shutdown()
{
	Remove();
}

void FPBLoadingScreenController::UpdateFadeIn()
{
	if (!LoadingScreenWidget.IsValid())
	{
		ClearFadeTimer();
		return;
	}

	FadeAlpha = FMath::Min(
		FadeAlpha + FadeUpdateIntervalSeconds / FadeDurationSeconds,
		1.0f);
	LoadingScreenWidget->SetRenderOpacity(FadeAlpha);
	if (FadeAlpha >= 1.0f)
	{
		ClearFadeTimer();
	}
}

void FPBLoadingScreenController::UpdateFadeOut()
{
	if (!LoadingScreenWidget.IsValid())
	{
		ClearFadeTimer();
		return;
	}

	FadeAlpha = FMath::Max(
		FadeAlpha - FadeUpdateIntervalSeconds / FadeDurationSeconds,
		0.0f);
	LoadingScreenWidget->SetLoadingContentOpacity(FadeAlpha);
	if (FadeAlpha <= 0.0f)
	{
		Remove();
	}
}

void FPBLoadingScreenController::Remove()
{
	ClearFadeTimer();
	if (LoadingScreenWidget.IsValid() && LoadingWorld.IsValid())
	{
		if (UGameViewportClient* GameViewport = LoadingWorld->GetGameViewport())
		{
			GameViewport->RemoveViewportWidgetContent(LoadingScreenWidget.ToSharedRef());
		}
	}

	LoadingScreenWidget.Reset();
	LoadingWorld.Reset();
	FadeAlpha = 0.0f;
}

void FPBLoadingScreenController::ClearFadeTimer()
{
	if (LoadingWorld.IsValid())
	{
		LoadingWorld->GetTimerManager().ClearTimer(FadeTimerHandle);
	}
}
