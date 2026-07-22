#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Ball/PBBallDetailTooltipViewData.h"
#include "PinBallLike/UI/Popup/PBSimplePopupWidget.h"
#include "PBBallRewardPopupWidget.generated.h"

class UPBBallDetailTooltipWidget;
class USizeBox;

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API UPBBallRewardPopupWidget : public UPBSimplePopupWidget
{
	GENERATED_BODY()

public:
	bool InitializeBallRewardPopup(
		const FText& InMessage,
		FName BallId,
		int32 StarLevel = 1);

private:
	bool BuildBallDetailViewData(
		FName BallId,
		int32 StarLevel,
		FPBBallDetailTooltipViewData& OutViewData) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball Reward|UI",
		meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPBBallDetailTooltipWidget> BallDetailTooltipClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> BallDetailContainer;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDetailTooltipWidget> BallDetailTooltip;
};
