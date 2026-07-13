#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBBettingWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnBetSelected,
	int32,
	SelectedIndex);

UCLASS()
class PINBALLLIKE_API UPBBettingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintAssignable, Category = "Betting")
	FOnBetSelected OnBetSelected;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> NationButton1;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> NationButton2;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> BetText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NationText1;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NationText2;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Flag1;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Flag2;

	UFUNCTION()
	void OnNationButton1Clicked();

	UFUNCTION()
	void OnNationButton2Clicked();

	UFUNCTION()
	void OnExitButtonClicked();
};