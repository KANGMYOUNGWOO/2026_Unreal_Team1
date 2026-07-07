#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBBossStatusWidget.generated.h"

class APBBossBase;
class UPBBossStatusViewModel;

UCLASS()
class PINBALLLIKE_API UPBBossStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Status UI")
	void SetBoss(APBBossBase* NewBoss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Status UI")
	void ClearBoss();

	UFUNCTION(BlueprintCallable, Category = "Boss|Enrage UI")
	void ShowEnrageWarning();

	UFUNCTION(BlueprintCallable, Category = "Boss|Enrage UI")
	void HideEnrageWarning();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Enrage UI", meta = (ClampMin = "0"))
	float EnrageWarningVisibleSeconds = 1.5f;

private:
	void EnsureStatusViewModel();
	bool ApplyViewModelToWidget();

	void ClearEnrageWarningTimer();

	UPROPERTY(Transient)
	TObjectPtr<UPBBossStatusViewModel> StatusViewModel;

	FTimerHandle EnrageWarningTimerHandle;
};
