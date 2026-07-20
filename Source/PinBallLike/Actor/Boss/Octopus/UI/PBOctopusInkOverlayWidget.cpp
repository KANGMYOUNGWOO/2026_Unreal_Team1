#include "PBOctopusInkOverlayWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

TSharedRef<SWidget> UPBOctopusInkOverlayWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}

	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* RootPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootPanel"));
		InkImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("InkImage"));
		WidgetTree->RootWidget = RootPanel;

		if (UCanvasPanelSlot* InkSlot = RootPanel->AddChildToCanvas(InkImage))
		{
			InkSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			InkSlot->SetOffsets(FMargin(0.0f));
		}

		InkImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else if (!InkImage)
	{
		InkImage = Cast<UImage>(WidgetTree->FindWidget(TEXT("InkImage")));
	}

	return Super::RebuildWidget();
}

void UPBOctopusInkOverlayWidget::ShowInk(UTexture2D* InkTexture, const float FadeInDuration)
{
	if (!InkImage || !InkTexture)
	{
		return;
	}

	InkImage->SetBrushFromTexture(InkTexture, true);
	FadeElapsedTime = 0.0f;
	FadeDuration = FMath::Max(0.0f, FadeInDuration);
	FadeState = FadeDuration > 0.0f ? EInkFadeState::FadingIn : EInkFadeState::None;
	SetRenderOpacity(FadeDuration > 0.0f ? 0.0f : 1.0f);
}

void UPBOctopusInkOverlayWidget::HideInk(const float FadeOutDuration)
{
	FadeElapsedTime = 0.0f;
	FadeDuration = FMath::Max(0.0f, FadeOutDuration);
	FadeOutStartOpacity = GetRenderOpacity();
	FadeState = FadeDuration > 0.0f ? EInkFadeState::FadingOut : EInkFadeState::None;

	if (FadeDuration <= 0.0f)
	{
		RemoveFromParent();
	}
}

void UPBOctopusInkOverlayWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	switch (FadeState)
	{
	case EInkFadeState::FadingIn:
		UpdateFadeIn(InDeltaTime);
		break;
	case EInkFadeState::FadingOut:
		UpdateFadeOut(InDeltaTime);
		break;
	default:
		break;
	}
}

void UPBOctopusInkOverlayWidget::UpdateFadeIn(const float InDeltaTime)
{
	FadeElapsedTime += InDeltaTime;
	const float FadeAlpha = FMath::Clamp(FadeElapsedTime / FadeDuration, 0.0f, 1.0f);
	SetRenderOpacity(FadeAlpha);

	if (FadeAlpha >= 1.0f)
	{
		FadeState = EInkFadeState::None;
	}
}

void UPBOctopusInkOverlayWidget::UpdateFadeOut(const float InDeltaTime)
{
	FadeElapsedTime += InDeltaTime;
	const float FadeAlpha = FMath::Clamp(FadeElapsedTime / FadeDuration, 0.0f, 1.0f);
	SetRenderOpacity(FMath::Lerp(FadeOutStartOpacity, 0.0f, FadeAlpha));

	if (FadeAlpha >= 1.0f)
	{
		FadeState = EInkFadeState::None;
		RemoveFromParent();
	}
}
