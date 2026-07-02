#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "Types/SlateEnums.h"
#include "PBCollectionWidget.generated.h"

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
 * 도감 프로토타입의 전체 화면 위젯입니다.
 * 카테고리 탭, 검색 / 필터 / 정렬, 카드 목록, 상세 패널, 시연용 상태 변경 버튼을 제공합니다.
 */
UCLASS()
class PINBALLLIKE_API UPBCollectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Collection|UI")
	void RefreshCollection();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BuildDefaultWidgetTree();
	void BuildHeader(UVerticalBox* RootBox);
	void BuildBody(UVerticalBox* RootBox);
	void BuildFilterPanel(UVerticalBox* ListBox);
	void BuildDetailPanel(UHorizontalBox* BodyBox);

	UButton* CreateTextButton(UWidgetTree* InWidgetTree, FName WidgetName, const FText& Label);
	UTextBlock* CreateText(UWidgetTree* InWidgetTree, FName WidgetName, int32 FontSize, const FLinearColor& Color);

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
	void HandleDiscoverClicked();
	UFUNCTION()
	void HandleUnlockClicked();
	UFUNCTION()
	void HandleCompleteClicked();
	UFUNCTION()
	void HandleResetClicked();
	UFUNCTION()
	void HandleEntryClicked(FName CollectionId);
	UFUNCTION()
	void HandleCollectionEntryChanged(FName CollectionId);

	UPROPERTY(EditDefaultsOnly, Category = "Collection|UI")
	TSubclassOf<UPBCollectionEntryWidget> EntryWidgetClass;

	UPROPERTY()
	TObjectPtr<UPBCollectionSubsystem> CollectionSubsystem;

	UPROPERTY()
	TObjectPtr<UUniformGridPanel> EntryGridPanel;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> SearchTextBox;

	UPROPERTY()
	TObjectPtr<UComboBoxString> AttackTypeComboBox;

	UPROPERTY()
	TObjectPtr<UComboBoxString> RoleComboBox;

	UPROPERTY()
	TObjectPtr<UComboBoxString> AttributeComboBox;

	UPROPERTY()
	TObjectPtr<UComboBoxString> StarGradeComboBox;

	UPROPERTY()
	TObjectPtr<UComboBoxString> SortModeComboBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> DetailNameText;

	UPROPERTY()
	TObjectPtr<UTextBlock> DetailMetaText;

	UPROPERTY()
	TObjectPtr<UTextBlock> DetailDescriptionText;

	UPROPERTY()
	TObjectPtr<UTextBlock> DetailUnlockText;

	UPROPERTY()
	TObjectPtr<UTextBlock> DetailRecordText;

	UPROPERTY()
	TObjectPtr<UBorder> DetailAccentBorder;

	UPROPERTY()
	TObjectPtr<UButton> AllTabButton;

	UPROPERTY()
	TObjectPtr<UButton> BallTabButton;

	UPROPERTY()
	TObjectPtr<UButton> BumperTabButton;

	UPROPERTY()
	TObjectPtr<UButton> BossTabButton;

	UPROPERTY()
	TObjectPtr<UButton> RelicTabButton;

	UPROPERTY()
	TObjectPtr<UButton> AchievementTabButton;

	UPROPERTY()
	TObjectPtr<UButton> DiscoverButton;

	UPROPERTY()
	TObjectPtr<UButton> UnlockButton;

	UPROPERTY()
	TObjectPtr<UButton> CompleteButton;

	UPROPERTY()
	TObjectPtr<UButton> ResetButton;

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
