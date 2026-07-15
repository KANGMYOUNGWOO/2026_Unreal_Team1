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

/**
 * 도감 목록에서 항목 하나의 표시 데이터와 클릭 전달을 담당합니다.
 * 외형은 이 클래스를 부모로 삼은 Widget Blueprint에서 구성합니다.
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

	UFUNCTION(BlueprintPure, Category = "Collection|Entry")
	UPBCollectionEntryViewModel* GetEntryViewModel() const { return EntryViewModel; }

protected:
	virtual void NativeOnInitialized() override;

	/** Widget Blueprint가 상태별 아이콘, 색상, 애니메이션을 갱신할 때 사용합니다. */
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

	/** WBP의 MVVM 바인딩이 참조하는 카드 표시 상태입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UPBCollectionEntryViewModel> EntryViewModel;

	bool bIsEntryViewModelApplied = false;
};
