#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PBBumperViewModel.generated.h"

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBBumperViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Bumper|ViewModel")
	void SetTriggerCount(int32 CurrentCount, int32 RequiredCount);

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Bumper|ViewModel")
	FText CurrentTriggerCountText = FText::AsNumber(0);

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Bumper|ViewModel")
	FText RequiredTriggerCountText = FText::AsNumber(0);
};
