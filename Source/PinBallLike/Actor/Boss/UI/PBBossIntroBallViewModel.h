#pragma once

#include "CoreMinimal.h"
#include "Components/SlateWrapperTypes.h"
#include "MVVMViewModelBase.h"
#include "Styling/SlateBrush.h"
#include "PBBossIntroBallViewModel.generated.h"

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBBossIntroBallViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Intro Ball ViewModel", meta = (WorldContext = "WorldContextObject"))
	void RefreshBallSprites(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Boss|Intro Ball ViewModel")
	void ClearBallSprites();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Intro Ball ViewModel")
	FSlateBrush Ball01Brush;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Intro Ball ViewModel")
	FSlateBrush Ball02Brush;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Intro Ball ViewModel")
	FSlateBrush Ball03Brush;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Intro Ball ViewModel")
	FText Ball01NameText = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Intro Ball ViewModel")
	FText Ball02NameText = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Intro Ball ViewModel")
	FText Ball03NameText = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Intro Ball ViewModel")
	ESlateVisibility Ball01Visibility = ESlateVisibility::Collapsed;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Intro Ball ViewModel")
	ESlateVisibility Ball02Visibility = ESlateVisibility::Collapsed;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Intro Ball ViewModel")
	ESlateVisibility Ball03Visibility = ESlateVisibility::Collapsed;

private:
	void ResolveBallViewData(
		const class UPBBallDeckSubsystem* BallDeckSubsystem,
		int32 SlotIndex,
		FSlateBrush& OutBallBrush,
		FText& OutBallNameText,
		ESlateVisibility& OutBallVisibility) const;
};
