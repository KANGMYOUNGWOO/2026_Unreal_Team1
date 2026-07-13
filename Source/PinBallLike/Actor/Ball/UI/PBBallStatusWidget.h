#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBBallStatusWidget.generated.h"

class APBBallBase;
class UPBBallStatusViewModel;
class UTexture2D;

UCLASS()
class PINBALLLIKE_API UPBBallStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ball|Status UI")
	void SetBall(APBBallBase* NewBall, UTexture2D* NewIconTexture);

	UFUNCTION(BlueprintCallable, Category = "Ball|Status UI")
	void SetIconTexture(UTexture2D* NewIconTexture);

	UFUNCTION(BlueprintCallable, Category = "Ball|Status UI")
	void ClearBall();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	void EnsureStatusViewModel();
	bool ApplyViewModelToWidget();

	UPROPERTY(Transient)
	TObjectPtr<UPBBallStatusViewModel> StatusViewModel;
};
