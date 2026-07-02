#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "PBCollectionEntryWidget.generated.h"

class UBorder;
class UButton;
class UTextBlock;
class UWidgetTree;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBCollectionEntryClickedSignature, FName, CollectionId);

/**
 * 도감 목록에서 항목 하나를 표시하는 카드 위젯입니다.
 * 더미 데이터 시연용 기본 UI를 C++에서 만들며, 이후 UMG 블루프린트로 외형을 교체할 수 있습니다.
 */
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

protected:
	virtual void NativeOnInitialized() override;

private:
	void BuildDefaultWidgetTree();
	void Refresh();

	UFUNCTION()
	void HandleClicked();

	static UTextBlock* CreateText(UWidgetTree* InWidgetTree, FName WidgetName, int32 FontSize);

	UPROPERTY()
	TObjectPtr<UButton> EntryButton;

	UPROPERTY()
	TObjectPtr<UBorder> CardBorder;

	UPROPERTY()
	TObjectPtr<UBorder> AccentBorder;

	UPROPERTY()
	TObjectPtr<UBorder> CategoryColorBorder;

	UPROPERTY()
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY()
	TObjectPtr<UTextBlock> MetaText;

	UPROPERTY()
	TObjectPtr<UTextBlock> DescriptionText;

	UPROPERTY()
	FPBCollectionDisplayData DisplayData;
};
