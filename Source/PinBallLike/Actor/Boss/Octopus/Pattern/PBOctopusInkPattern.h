#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossPatternBase.h"
#include "PBOctopusInkPattern.generated.h"

class UPBOctopusInkOverlayWidget;
class UAnimSequence;
class UTexture2D;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBOctopusInkPattern : public UPBBossPatternBase
{
	GENERATED_BODY()

public:
	UPBOctopusInkPattern();

protected:
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	virtual void ExecutePattern_Implementation(APBBossBase* Boss) override;
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Ink")
	TSubclassOf<UPBOctopusInkOverlayWidget> InkWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Ink")
	TObjectPtr<UTexture2D> InkTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Ink")
	TObjectPtr<UAnimSequence> InkAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Ink", meta = (ClampMin = "0.0"))
	float InkStartDelay = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Ink", meta = (ClampMin = "0.0"))
	float InkDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Ink", meta = (ClampMin = "0.0"))
	float FadeInDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Ink", meta = (ClampMin = "0.0"))
	float FadeOutDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Ink")
	int32 InkZOrder = 100;

private:
	void StartInk();
	void BeginFadeOut();
	void CompleteInkPattern();
	void CleanupInkPattern();
	void ClearInkTimers();

	UPROPERTY(Transient)
	TObjectPtr<UPBOctopusInkOverlayWidget> InkWidget;

	FTimerHandle InkStartTimerHandle;
	FTimerHandle FadeOutStartTimerHandle;
	FTimerHandle PatternFinishTimerHandle;
};
