#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "PBCollectionEntryWidget.generated.h"

class UBorder;
class UButton;
class UTextBlock;
class UPBCollectionEntryViewModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBCollectionEntryClickedSignature, FName, CollectionId);

UCLASS()
class PINBALLLIKE_API UPBCollectionEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Collection|Event")
	FPBCollectionEntryClickedSignature OnEntryClicked;

	UFUNCTION(BlueprintCallable, Category = "Collection|Entry")
	void SetDisplayData(const FPBCollectionDisplayData& InDisplayData);

	UFUNCTION(BlueprintPure, Category = "Collection|Entry")
	FName GetCollectionId() const { return DisplayData.CollectionId; }

	UFUNCTION(BlueprintPure, Category = "Collection|Entry")
	UPBCollectionEntryViewModel* GetEntryViewModel() const { return EntryViewModel; }

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Collection|Entry", meta = (DisplayName = "On Collection Display Data Changed"))
	void BP_OnCollectionDisplayDataChanged(const FPBCollectionDisplayData& InDisplayData);

private:
	void EnsureEntryViewModel();
	bool ApplyViewModelToWidget();
	void Refresh();
	bool ValidateRequiredWidgetBindings() const;

	UFUNCTION()
	void HandleClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> EntryButton;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Entry", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UBorder> CardBorder;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Entry", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UBorder> AccentBorder;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Entry", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UBorder> CategoryColorBorder;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Entry", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Entry", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> MetaText;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Entry", meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> DescriptionText;

	UPROPERTY(Transient)
	FPBCollectionDisplayData DisplayData;

	UPROPERTY(Transient)
	TObjectPtr<UPBCollectionEntryViewModel> EntryViewModel;

	bool bIsEntryViewModelApplied = false;
};
