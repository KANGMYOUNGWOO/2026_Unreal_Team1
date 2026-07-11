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
class UHorizontalBox;
class UPBCollectionEntryWidget;
class UPBCollectionSubsystem;
class UScrollBox;
class UTextBlock;
class UUniformGridPanel;
class UVerticalBox;
class UWidgetTree;

/**
 * 도감 전체 화면의 데이터 처리와 사용자 입력을 담당합니다.
 * 외형은 이 클래스를 부모로 삼은 Widget Blueprint에서 구성하며, C++ UI는 기존 클래스 직접 참조를 위한 호환 폴백입니다.
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
	void BuildDefaultWidgetTree();
	void BuildHeader(UVerticalBox* RootBox);
	void BuildBody(UVerticalBox* RootBox);
	void BuildFilterPanel(UVerticalBox* ListBox);
	void BuildDetailPanel(UHorizontalBox* BodyBox);

	UButton* CreateTextButton(UWidgetTree* InWidgetTree, FName WidgetName, const FText& Label);
	UTextBlock* CreateText(UWidgetTree* InWidgetTree, FName WidgetName, int32 FontSize, const FLinearColor& Color);
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collection|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPBCollectionEntryWidget> EntryWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UPBCollectionSubsystem> CollectionSubsystem;

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

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DetailNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DetailMetaText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DetailDescriptionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DetailUnlockText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DetailRecordText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> DetailAccentBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AllTabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BallTabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BumperTabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BossTabButton;

	UPROPERTY(meta = (BindWidgetOptional))
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
	bool bUsesRuntimeFallbackTree = false;
	FName SelectedCollectionId = NAME_None;
};
