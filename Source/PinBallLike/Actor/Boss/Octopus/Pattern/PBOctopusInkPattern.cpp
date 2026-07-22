#include "PBOctopusInkPattern.h"

#include "Animation/AnimSequence.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Octopus/PBOctopusBoss.h"
#include "PinBallLike/Actor/Boss/Octopus/UI/PBOctopusInkOverlayWidget.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

UPBOctopusInkPattern::UPBOctopusInkPattern()
{
	InkWidgetClass = UPBOctopusInkOverlayWidget::StaticClass();

	static ConstructorHelpers::FObjectFinder<UTexture2D> InkTextureFinder(
		TEXT("/Game/Blueprints/Boss/BossAsset/dark_image-removebg-preview.dark_image-removebg-preview"));
	if (InkTextureFinder.Succeeded())
	{
		InkTexture = InkTextureFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> InkAnimationFinder(
		TEXT("/Game/Blueprints/Boss/BossAsset/pulbo-monstruo/source/Boss8Armature_Atacar.Boss8Armature_Atacar"));
	if (InkAnimationFinder.Succeeded())
	{
		InkAnimation = InkAnimationFinder.Object;
	}
}

bool UPBOctopusInkPattern::CanExecute_Implementation(APBBossBase* Boss) const
{
	return Super::CanExecute_Implementation(Boss)
		&& InkWidgetClass
		&& InkTexture
		&& InkAnimation;
}

void UPBOctopusInkPattern::ExecutePattern_Implementation(APBBossBase* Boss)
{
	APBOctopusBoss* OctopusBoss = Cast<APBOctopusBoss>(Boss);
	if (!OctopusBoss || !InkWidgetClass || !InkTexture || !InkAnimation)
	{
		FinishPattern();
		return;
	}

	CleanupInkPattern();
	OctopusBoss->PlayOctopusAnimation(InkAnimation);

	if (InkStartDelay <= 0.0f)
	{
		StartInk();
		return;
	}

	OctopusBoss->GetWorldTimerManager().SetTimer(
		InkStartTimerHandle,
		this,
		&UPBOctopusInkPattern::StartInk,
		InkStartDelay,
		false);
}

void UPBOctopusInkPattern::StartInk()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		CompleteInkPattern();
		return;
	}

	PlayPatternSFX(Boss);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(Boss, 0);
	if (!PlayerController)
	{
		CompleteInkPattern();
		return;
	}

	InkWidget = CreateWidget<UPBOctopusInkOverlayWidget>(PlayerController, InkWidgetClass);
	if (!InkWidget)
	{
		FinishPattern();
		return;
	}

	InkWidget->AddToViewport(InkZOrder);
	InkWidget->ShowInk(InkTexture, FadeInDuration);

	const float FadeOutStartDelay = FadeInDuration + InkDuration;
	if (FadeOutStartDelay <= 0.0f)
	{
		BeginFadeOut();
		return;
	}

	Boss->GetWorldTimerManager().SetTimer(
		FadeOutStartTimerHandle,
		this,
		&UPBOctopusInkPattern::BeginFadeOut,
		FadeOutStartDelay,
		false);
}

void UPBOctopusInkPattern::CancelPatternInternal_Implementation(APBBossBase* Boss)
{
	static_cast<void>(Boss);
	CleanupInkPattern();
}

void UPBOctopusInkPattern::BeginFadeOut()
{
	if (!InkWidget)
	{
		CompleteInkPattern();
		return;
	}

	InkWidget->HideInk(FadeOutDuration);
	if (FadeOutDuration <= 0.0f)
	{
		CompleteInkPattern();
		return;
	}

	if (APBBossBase* Boss = GetOwnerBoss())
	{
		Boss->GetWorldTimerManager().SetTimer(
			PatternFinishTimerHandle,
			this,
			&UPBOctopusInkPattern::CompleteInkPattern,
			FadeOutDuration,
			false);
		return;
	}

	CompleteInkPattern();
}

void UPBOctopusInkPattern::CompleteInkPattern()
{
	CleanupInkPattern();
	FinishPattern();
}

void UPBOctopusInkPattern::CleanupInkPattern()
{
	ClearInkTimers();

	if (APBOctopusBoss* OctopusBoss = Cast<APBOctopusBoss>(GetOwnerBoss()))
	{
		OctopusBoss->RestoreOctopusAnimationMode();
	}

	if (InkWidget)
	{
		InkWidget->RemoveFromParent();
	}

	InkWidget = nullptr;
}

void UPBOctopusInkPattern::ClearInkTimers()
{
	if (APBBossBase* Boss = GetOwnerBoss())
	{
		FTimerManager& TimerManager = Boss->GetWorldTimerManager();
		TimerManager.ClearTimer(InkStartTimerHandle);
		TimerManager.ClearTimer(FadeOutStartTimerHandle);
		TimerManager.ClearTimer(PatternFinishTimerHandle);
	}
}
