#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "UObject/Object.h"
#include "PBBumperUIPresenter.generated.h"

class UPBBumperViewModel;
class UUserWidget;


//TODO : MVVM 인데 presenter가 왜 필요하지? 다른 방향의 사용법 구상하자.

UCLASS(BlueprintType)
class PINBALLLIKE_API UPBBumperUIPresenter : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Bumper|UI")
	bool Bind(APBModularBumperBase* InBumper, UUserWidget* InWidget);

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

	TWeakObjectPtr<APBModularBumperBase> Bumper;
};
