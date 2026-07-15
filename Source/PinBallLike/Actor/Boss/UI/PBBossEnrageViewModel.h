#pragma once

#include "CoreMinimal.h"
#include "Components/SlateWrapperTypes.h"
#include "MVVMViewModelBase.h"
#include "PBBossEnrageViewModel.generated.h"

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBBossEnrageViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Enrage ViewModel")
	void ShowEnrageWarning();

	UFUNCTION(BlueprintCallable, Category = "Boss|Enrage ViewModel")
	void HideEnrageWarning();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Enrage ViewModel")
	ESlateVisibility EnrageWarningVisibility = ESlateVisibility::Collapsed;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Enrage ViewModel")
	FText EnrageWarningText = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Enrage ViewModel")
	float VignetteOpacity = 0.0f;
};
