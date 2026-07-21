#include "PBLoadingScreenController.h"

#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "PinBallLike/UI/Loading/PBLoadingScreen.h"
#include "SLoadingScreenLayout.h"

void FPBLoadingScreenController::Show(UWorld* World)
{
	if (!EnsureLoadingScreen(World))
	{
		return;
	}

	ClearFadeTimer();
	FadeAlpha = 1.0f;
	FadeState = EFadeState::Visible;
	LoadingScreenWidget->SetRenderOpacity(FadeAlpha);
}

bool FPBLoadingScreenController::EnsureLoadingScreen(UWorld* World)
{
	if (LoadingScreenWidget.IsValid())
	{
		return LoadingWorld.IsValid();
	}

	if (!World || !World->GetGameViewport())
	{
		return false;
	}

	LoadingWorld = World;
	LoadingScreenWidget = FPBLoadingScreen::CreateLoadingScreenWidget();
	World->GetGameViewport()->AddViewportWidgetContent(LoadingScreenWidget.ToSharedRef(), MAX_int32);
	return true;
}

void FPBLoadingScreenController::Hide()
{
	if (!LoadingScreenWidget.IsValid()
		|| !LoadingWorld.IsValid()
		|| FadeState == EFadeState::FadingOut)
	{
		return;
	}

	StartFadeOut();
}

void FPBLoadingScreenController::Shutdown()
{
	Remove();
}

void FPBLoadingScreenController::StartFadeOut()
{
	if (!LoadingWorld.IsValid())
	{
		return;
	}


	ClearFadeTimer();
	FadeState = EFadeState::FadingOut;
	LoadingWorld->GetTimerManager().SetTimer(
		FadeTimerHandle,
		FTimerDelegate::CreateRaw(this, &FPBLoadingScreenController::UpdateFadeOut),
		FadeUpdateIntervalSeconds,
		true);
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
	LoadingScreenWidget->SetRenderOpacity(FadeAlpha);

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
	FadeState = EFadeState::Hidden;
}

void FPBLoadingScreenController::ClearFadeTimer()
{
	if (LoadingWorld.IsValid())
	{
		LoadingWorld->GetTimerManager().ClearTimer(FadeTimerHandle);
	}
}
