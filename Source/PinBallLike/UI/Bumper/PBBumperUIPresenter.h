#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PBBumperUIPresenter.generated.h"

class APBBumperBase;
class UPBBumperViewModel;
class UUserWidget;

UCLASS(BlueprintType)
class PINBALLLIKE_API UPBBumperUIPresenter : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Bumper|UI")
	bool Bind(APBBumperBase* InBumper, UUserWidget* InWidget);

	UFUNCTION(BlueprintCallable, Category = "Bumper|UI")
	void Unbind();

	UFUNCTION(BlueprintPure, Category = "Bumper|UI")
	UPBBumperViewModel* GetBumperViewModel() const { return BumperViewModel; }

protected:
	virtual void BeginDestroy() override;

private:
	bool ApplyViewModelToWidget(UUserWidget* Widget);

	UFUNCTION()
	void HandleTriggerCountChanged(int32 CurrentCount, int32 RequiredCount);

	UPROPERTY(Transient)
	TObjectPtr<UPBBumperViewModel> BumperViewModel;

	TWeakObjectPtr<APBBumperBase> Bumper;
};
