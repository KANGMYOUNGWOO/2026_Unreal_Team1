#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Synergy/PBSynergyViewData.h"
#include "PBSynergyBallIconWidget.generated.h"

UCLASS()
class PINBALLLIKE_API UPBSynergyBallIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Synergy|BallIcon")
	void InitializeBallIcon(const FPBSynergyBallIconViewData& InViewData);

private:
	void ApplyIcon();

	UPROPERTY(Transient)
	FPBSynergyBallIconViewData ViewData;
};
