#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "UObject/Object.h"
#include "PBBumperUIBinder.generated.h"

class UPBBumperViewModel;
class UUserWidget;

/** 런타임 범퍼 이벤트를 MVVM ViewModel에 전달하고 위젯 생명주기에 맞춰 해제합니다. */
UCLASS(BlueprintType)
class PINBALLLIKE_API UPBBumperUIBinder : public UObject
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
