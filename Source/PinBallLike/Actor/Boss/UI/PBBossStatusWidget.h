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

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	void EnsureStatusViewModel();
	bool ApplyViewModelToWidget();

	UPROPERTY(Transient)
	TObjectPtr<UPBBossStatusViewModel> StatusViewModel;
};
