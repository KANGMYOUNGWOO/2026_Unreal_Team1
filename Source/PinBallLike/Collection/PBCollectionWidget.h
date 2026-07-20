#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "PinBallLike/UI/PBUserWidget.h"
#include "PBCollectionWidget.generated.h"

class APlayerController;
class UButton;
class UPBCollectionTabWidgetBase;
class UWidgetSwitcher;

UCLASS()
class PINBALLLIKE_API UPBCollectionWidget : public UPBUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Collection|UI")
	void RefreshCollection();

	UFUNCTION(BlueprintCallable, Category = "Collection|UI")
	void ShowCollectionTab(EPBCollectionCategory Category);

	UFUNCTION(BlueprintPure, Category = "Collection|UI")
	EPBCollectionCategory GetCurrentCategory() const { return CurrentCategory; }

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	virtual void OnPushed_Implementation() override;
	virtual void OnPopped_Implementation() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Collection|UI", meta = (DisplayName = "On Collection Category Changed"))
	void BP_OnCollectionCategoryChanged(EPBCollectionCategory NewCategory);

private:
	bool RedirectLegacyNativeWidget();
	void BindWidgetEvents();
	void UnbindWidgetEvents();
	void ApplyTabButtonStyles();
	bool ValidateRequiredWidgetBindings() const;
	UPBCollectionTabWidgetBase* ResolveTabWidget(EPBCollectionCategory Category) const;
	APlayerController* ResolvePlayerController() const;
	void ApplyCollectionInputMode(bool bEnableUI) const;

	UFUNCTION() void HandleBallTabClicked();
	UFUNCTION() void HandleSynergyTabClicked();
	UFUNCTION() void HandleRelicTabClicked();
	UFUNCTION() void HandleBumperTabClicked();
	UFUNCTION() void HandleBossTabClicked();
	UFUNCTION() void HandleCloseClicked();

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UWidgetSwitcher> TabSwitcher;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BallTabButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> SynergyTabButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> RelicTabButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BumperTabButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BossTabButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPBCollectionTabWidgetBase> BallTabContent;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPBCollectionTabWidgetBase> SynergyTabContent;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPBCollectionTabWidgetBase> RelicTabContent;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPBCollectionTabWidgetBase> BumperTabContent;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPBCollectionTabWidgetBase> BossTabContent;

	EPBCollectionCategory CurrentCategory = EPBCollectionCategory::Ball;
};
