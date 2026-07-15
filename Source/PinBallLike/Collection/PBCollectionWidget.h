#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "PinBallLike/UI/PBUserWidget.h"
#include "Types/SlateEnums.h"
#include "PBCollectionWidget.generated.h"

class APlayerController;
class UBorder;
class UButton;
class UComboBoxString;
class UEditableTextBox;
class UPBCollectionDetailViewModel;
class UPBCollectionEntryWidget;
class UPBCollectionSubsystem;
class UTextBlock;
class UUniformGridPanel;

/**
 * 도감 전체 화면의 데이터 처리와 사용자 입력을 담당합니다.
 * 외형은 이 클래스를 부모로 삼은 Widget Blueprint에서 구성합니다.
 */
UCLASS()
class PINBALLLIKE_API UPBCollectionWidget : public UPBUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Collection|UI")
	void RefreshCollection();

	UFUNCTION(BlueprintPure, Category = "Collection|UI")
	EPBCollectionCategory GetCurrentCategory() const { return CurrentCategory; }

	UFUNCTION(BlueprintPure, Category = "Collection|UI")
	FName GetSelectedCollectionId() const { return SelectedCollectionId; }

	UFUNCTION(BlueprintPure, Category = "Collection|UI")
	UPBCollectionDetailViewModel* GetDetailViewModel() const { return DetailViewModel; }

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void OnPushed_Implementation() override;
	virtual void OnPopped_Implementation() override;

	/** Widget Blueprint가 탭 선택 색상이나 전환 애니메이션을 갱신할 때 사용합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Collection|UI", meta = (DisplayName = "On Collection Category Changed"))
	void BP_OnCollectionCategoryChanged(EPBCollectionCategory NewCategory);

private:
	/** 메인 메뉴에 남은 C++ 원본 클래스 참조를 실제 도감 WBP로 교체하는 임시 호환 경로입니다. */
	bool RedirectLegacyNativeWidget();
	void BindWidgetEvents();
	bool ValidateRequiredWidgetBindings() const;

	void PopulateFilterOptions();
	void PopulateMetadataComboBox(
		UComboBoxString* ComboBox,
		const TArray<FName>& OptionIds,
		const FText& AllOptionText) const;
	FName ResolveMetadataSelection(const FString& SelectedItem, const TArray<FName>& OptionIds) const;
	void RefreshCollectionByFilterChange();

	void RefreshEntryList();
	void RefreshDetail();
	void EnsureDetailViewModel();
	bool ApplyDetailViewModelToWidget();
	void SetCategory(EPBCollectionCategory NewCategory);
	void SelectEntry(FName CollectionId);
	APlayerController* ResolvePlayerController() const;
	void ApplyCollectionInputMode(bool bEnableUI) const;

	UFUNCTION()
	void HandleAllTabClicked();
	UFUNCTION()
	void HandleBallTabClicked();
	UFUNCTION()
	void HandleBumperTabClicked();
	UFUNCTION()
	void HandleBossTabClicked();
	UFUNCTION()
	void HandleRelicTabClicked();
	UFUNCTION()
	void HandleAchievementTabClicked();
	UFUNCTION()
	void HandleCloseClicked();
	UFUNCTION()
	void HandleSearchTextChanged(const FText& Text);
	UFUNCTION()
	void HandleAttackTypeFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void HandleRoleFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void HandleAttributeFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void HandleStarGradeFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void HandleSortModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void HandleEntryClicked(FName CollectionId);
	UFUNCTION()
	void HandleCollectionEntryChanged(FName CollectionId);
	UFUNCTION()
	void HandleCollectionDataReady(bool bIsReady);

	/** 목록 항목용 Widget Blueprint입니다. C++ 원본 클래스가 아니라 실제 BP 클래스를 지정해야 합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collection|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPBCollectionEntryWidget> EntryWidgetClass;

	/** 한 줄에 배치할 도감 카드 수입니다. 카드 자체 크기는 Entry Widget Blueprint에서 정합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collection|UI|Layout", meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 EntryColumnCount = 3;

	/** 도감 카드 사이의 전체 여백입니다. 각 Grid Slot에는 절반씩 적용됩니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collection|UI|Layout", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float EntrySpacing = 16.0f;

	UPROPERTY(Transient)
	TObjectPtr<UPBCollectionSubsystem> CollectionSubsystem;

	/** 선택 항목의 상세 표시 상태입니다. 저장 데이터와는 분리된 일시적 UI 객체입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UPBCollectionDetailViewModel> DetailViewModel;

	bool bIsDetailViewModelApplied = false;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> EntryGridPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> SearchTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> AttackTypeComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> RoleComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> AttributeComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> StarGradeComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> SortModeComboBox;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Detail", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> DetailNameText;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Detail", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> DetailMetaText;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Detail", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> DetailDescriptionText;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Detail", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> DetailUnlockText;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Detail", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> DetailRecordText;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Detail", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UBorder> DetailAccentBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AllTabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BallTabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BumperTabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BossTabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RelicTabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AchievementTabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	EPBCollectionCategory CurrentCategory = EPBCollectionCategory::All;
	FPBCollectionQuery CurrentQuery;
	TArray<FName> AttackTypeFilterIds;
	TArray<FName> RoleFilterIds;
	TArray<FName> AttributeFilterIds;
	TMap<FString, int32> StarGradeFilterMap;
	TMap<FString, EPBCollectionSortMode> SortModeMap;
	bool bIsPopulatingFilterOptions = false;
	FName SelectedCollectionId = NAME_None;
};
