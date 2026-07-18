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

/**
 * 도감 탭의 검색어, 선택 행, 목록 객체 수명과 데이터 변경 구독을 담당합니다.
 * 실제 위젯 배치와 카테고리별 상세 표현은 소유하지 않습니다.
 */
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
	bool bInitialized = false;
	bool bIsActive = false;
};
