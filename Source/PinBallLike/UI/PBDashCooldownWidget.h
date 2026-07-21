#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBDashCooldownWidget.generated.h"

class UImage;
class UMaterialInstanceDynamic;
class UTextBlock;

UCLASS()
class PINBALLLIKE_API UPBDashCooldownWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DashCooldown")
	FName CooldownRatioParameterName = TEXT("Percent");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DashCooldown", meta = (ClampMin = "0.0"))
	float ReadyRatioValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DashCooldown")
	bool bHideRemainingTimeWhenReady = true;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_CooldownCircle;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RemainingTime;

private:
	void RefreshCooldown();
	void SetCooldownRatio(float CooldownRatio);
	void SetRemainingTimeText(float RemainingSeconds);

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CooldownCircleMaterial;
};
