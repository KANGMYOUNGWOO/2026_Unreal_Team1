#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBGolemHandStatusWidget.generated.h"

class APBGolemBoss;
class UPBGolemHandStatusViewModel;

UCLASS()
class PINBALLLIKE_API UPBGolemHandStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand UI")
	void SetGolemBoss(APBGolemBoss* NewGolemBoss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand UI")
	void ClearGolemBoss();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	void EnsureViewModel();
	bool ApplyViewModelToWidget();

	UPROPERTY(Transient)
	TObjectPtr<UPBGolemHandStatusViewModel> HandStatusViewModel;
};
