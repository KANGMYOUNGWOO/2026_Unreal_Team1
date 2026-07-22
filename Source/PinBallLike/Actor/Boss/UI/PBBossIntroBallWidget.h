#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBBossIntroBallWidget.generated.h"

class UPBBossIntroBallViewModel;

UCLASS()
class PINBALLLIKE_API UPBBossIntroBallWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Intro Ball UI")
	UPBBossIntroBallViewModel* CreateBallIntroViewModel();

	UFUNCTION(BlueprintCallable, Category = "Boss|Intro Ball UI")
	void SetBallIntroViewModel(UPBBossIntroBallViewModel* NewBallIntroViewModel);

	UFUNCTION(BlueprintPure, Category = "Boss|Intro Ball UI")
	UPBBossIntroBallViewModel* GetBallIntroViewModel() const;

protected:
	virtual void NativeOnInitialized() override;

private:
	bool ApplyViewModelToWidget();

	UPROPERTY(Transient)
	TObjectPtr<UPBBossIntroBallViewModel> BallIntroViewModel;
};
