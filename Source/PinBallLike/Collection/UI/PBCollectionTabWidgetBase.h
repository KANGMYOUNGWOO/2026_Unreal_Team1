#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Collection/PBCollectionTabTypes.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "PBCollectionTabWidgetBase.generated.h"

class UEditableTextBox;
class UListView;
class UPBCollectionCatalogItemObject;
class UPBCollectionSubsystem;
class UTextBlock;

/**
 * 탭 활성화, 검색 입력, 공통 목록 아이템 수명만 담당합니다.
 * 탭마다 다른 레이아웃과 상세 데이터 해석은 파생 클래스가 소유합니다.
 */
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
	virtual void NativeDestruct() override;

	UPBCollectionSubsystem* GetCollectionSubsystem() const { return CollectionSubsystem; }
	bool IsCatalogDataReady(const FText& LoadingText) const;
	FString GetNormalizedSearchText() const;
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

	UFUNCTION()
	void HandleCollectionDataReady(bool bReady);

	UFUNCTION()
	void HandleCollectionEntryChanged(FName CollectionId);

	UPROPERTY(Transient)
	TObjectPtr<UPBCollectionSubsystem> CollectionSubsystem;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBCollectionCatalogItemObject>> CatalogItems;

	FName SelectedSourceRowName = NAME_None;
	bool bIsActiveTab = false;
};
