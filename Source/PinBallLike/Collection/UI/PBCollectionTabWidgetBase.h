#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Collection/PBCollectionTabTypes.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "PBCollectionTabWidgetBase.generated.h"

class UEditableTextBox;
class UListView;
class UPBCollectionCatalogItemObject;
class UPBCollectionTabController;
class UPBCollectionSubsystem;
class UTextBlock;

UCLASS(Abstract)
class PINBALLLIKE_API UPBCollectionTabWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Collection|Tab")
	void ActivateTab();

	UFUNCTION(BlueprintCallable, Category = "Collection|Tab")
	void DeactivateTab();

	UFUNCTION(BlueprintCallable, Category = "Collection|Tab")
	virtual void RefreshTab();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPBCollectionSubsystem* GetCollectionSubsystem() const;
	bool IsCatalogDataReady(const FText& LoadingText) const;
	bool PrepareCatalogRefresh(UListView* ListView, const FText& LoadingText) const;
	bool MatchesSearch(const FPBCollectionItemSummary& Summary, const TArray<FText>& AdditionalTexts = {}) const;
	int32 PopulateCatalogItems(
		UListView* ListView,
		EPBCollectionCategory Category,
		const TArray<FPBCollectionItemSummary>& Summaries,
		const TArray<int32>& DataIndexes);
	const UPBCollectionCatalogItemObject* ResolveCatalogItem(UObject* ItemObject);
	void SetStatus(const FText& Text, bool bShow) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Collection|Tab", meta = (DisplayName = "On Collection Tab Activated"))
	void BP_OnTabActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Collection|Tab", meta = (DisplayName = "On Collection Tab Deactivated"))
	void BP_OnTabDeactivated();

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Tab", meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> SearchTextBox;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Tab", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusText;

private:
	UFUNCTION()
	void HandleSearchTextChanged(const FText& Text);

	void HandleRefreshRequested();

	UPROPERTY(Transient)
	TObjectPtr<UPBCollectionTabController> TabController;
};
