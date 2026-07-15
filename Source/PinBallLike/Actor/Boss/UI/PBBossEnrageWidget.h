#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBBossEnrageWidget.generated.h"

class UPBBossEnrageViewModel;

UCLASS()
class PINBALLLIKE_API UPBBossEnrageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
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
	void EnsureEnrageViewModel();
	bool ApplyViewModelToWidget();
	void ClearEnrageWarningTimer();

	UPROPERTY(Transient)
	TObjectPtr<UPBBossEnrageViewModel> EnrageViewModel;

	FTimerHandle EnrageWarningTimerHandle;
};
