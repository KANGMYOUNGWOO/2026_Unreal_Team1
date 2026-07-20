#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Collection/PBCollectionTabTypes.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "UObject/Object.h"
#include "PBCollectionTabController.generated.h"

class UGameInstance;
class UPBCollectionCatalogItemObject;
class UPBCollectionSubsystem;

DECLARE_MULTICAST_DELEGATE(FPBCollectionTabRefreshRequestedNative);

UCLASS()
class PINBALLLIKE_API UPBCollectionTabController : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UGameInstance* InGameInstance);
	void Shutdown();
	void Activate();
	void Deactivate();
	void SetSearchText(const FText& Text);
	void SetSortMode(EPBCollectionSortMode InSortMode);
	EPBCollectionSortMode GetSortMode() const { return SortMode; }

	UPBCollectionSubsystem* GetCollectionSubsystem() const { return CollectionSubsystem; }
	bool IsCatalogDataReady() const;
	bool HasDataLoadCompleted() const;
	bool MatchesSearch(
		const FPBCollectionItemSummary& Summary,
		const TArray<FText>& AdditionalTexts = {}) const;
	int32 BuildCatalogItems(
		EPBCollectionCategory Category,
		const TArray<FPBCollectionItemSummary>& Summaries,
		const TArray<int32>& DataIndexes,
		TArray<UPBCollectionCatalogItemObject*>& OutItems,
		UPBCollectionCatalogItemObject*& OutSelectedItem);
	const UPBCollectionCatalogItemObject* ResolveCatalogItem(UObject* ItemObject);

	FPBCollectionTabRefreshRequestedNative OnRefreshRequested;

private:
	UFUNCTION()
	void HandleCollectionDataReady(bool bReady);

	UFUNCTION()
	void HandleCollectionEntryChanged(FName CollectionId);

	UPROPERTY(Transient)
	TObjectPtr<UPBCollectionSubsystem> CollectionSubsystem;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBCollectionCatalogItemObject>> CatalogItems;

	FString NormalizedSearchText;
	FName SelectedSourceRowName = NAME_None;
	EPBCollectionSortMode SortMode = EPBCollectionSortMode::SortOrder;
	bool bInitialized = false;
	bool bIsActive = false;
};
