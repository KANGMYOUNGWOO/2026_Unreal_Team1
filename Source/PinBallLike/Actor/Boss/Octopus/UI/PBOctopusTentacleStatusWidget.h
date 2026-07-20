#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBOctopusTentacleStatusWidget.generated.h"

class APBOctopusBoss;
class UPBOctopusTentacleStatusViewModel;

UCLASS()
class PINBALLLIKE_API UPBOctopusTentacleStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Octopus Tentacle UI")
	void SetOctopusBoss(APBOctopusBoss* NewOctopusBoss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Octopus Tentacle UI")
	void ClearOctopusBoss();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	void EnsureViewModel();
	bool ApplyViewModelToWidget();

	UPROPERTY(Transient)
	TObjectPtr<UPBOctopusTentacleStatusViewModel> TentacleStatusViewModel;
};
