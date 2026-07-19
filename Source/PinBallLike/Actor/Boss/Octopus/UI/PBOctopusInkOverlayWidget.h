#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBOctopusInkOverlayWidget.generated.h"

class UImage;
class UTexture2D;

UCLASS()
class PINBALLLIKE_API UPBOctopusInkOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowInk(UTexture2D* InkTexture, float FadeInDuration);
	void HideInk(float FadeOutDuration);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	enum class EInkFadeState : uint8
	{
		None,
		FadingIn,
		FadingOut
	};

	void UpdateFadeIn(float InDeltaTime);
	void UpdateFadeOut(float InDeltaTime);

	UPROPERTY(Transient)
	TObjectPtr<UImage> InkImage;

	EInkFadeState FadeState = EInkFadeState::None;
	float FadeElapsedTime = 0.0f;
	float FadeDuration = 0.0f;
	float FadeOutStartOpacity = 1.0f;
};
