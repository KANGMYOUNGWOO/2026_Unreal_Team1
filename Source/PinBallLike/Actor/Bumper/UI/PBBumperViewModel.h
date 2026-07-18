#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PBBumperViewModel.generated.h"

/** 범퍼별 현재 카운트와 요구 카운트를 FieldNotify 값으로 노출합니다. */
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
