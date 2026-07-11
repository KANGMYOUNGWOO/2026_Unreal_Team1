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
 * 도감 목록에서 항목 하나의 표시 데이터와 클릭 전달을 담당합니다.
 * 외형은 이 클래스를 부모로 삼은 Widget Blueprint에서 구성하며, C++ UI는 클래스 직접 생성 시 사용하는 호환 폴백입니다.
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

	/** Widget Blueprint가 상태별 아이콘, 색상, 애니메이션을 갱신할 때 사용합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Collection|Entry", meta = (DisplayName = "On Collection Display Data Changed"))
	void BP_OnCollectionDisplayDataChanged(const FPBCollectionDisplayData& InDisplayData);

private:
	void BuildDefaultWidgetTree();
	void Refresh();
	bool ValidateRequiredWidgetBindings() const;

	UFUNCTION()
	void HandleClicked();

	static UTextBlock* CreateText(UWidgetTree* InWidgetTree, FName WidgetName, int32 FontSize);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> EntryButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> CardBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> AccentBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> CategoryColorBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MetaText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DescriptionText;

	UPROPERTY(Transient)
	FPBCollectionDisplayData DisplayData;
};
