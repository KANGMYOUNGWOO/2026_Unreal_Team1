#include "PBCollectionWidget.h"

#include "PBCollectionEntryWidget.h"
#include "PinBallLike/Collection/PBCollectionSubsystem.h"
#include "PinBallLike/Subsystem/PBUIManagerSubsystem.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "UObject/SoftObjectPath.h"

namespace
{
const FSoftClassPath LegacyCollectionWidgetBlueprintPath(
	TEXT("/Game/Blueprints/UI/Collection/WBP_CollectionWidget.WBP_CollectionWidget_C"));
}

void UPBCollectionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// WBP_MainMenu의 기존 C++ 클래스 참조는 OnPushed에서 실제 WBP로 교체합니다.
	if (GetClass() == StaticClass())
	{
		return;
	}

	BindWidgetEvents();
	ValidateRequiredWidgetBindings();
}

void UPBCollectionWidget::BindWidgetEvents()
{
	if (AllTabButton)
	{
		AllTabButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionWidget::HandleAllTabClicked);
	}
	if (BallTabButton)
	{
		BallTabButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionWidget::HandleBallTabClicked);
	}
	if (BumperTabButton)
	{
		BumperTabButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionWidget::HandleBumperTabClicked);
	}
	if (BossTabButton)
	{
		BossTabButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionWidget::HandleBossTabClicked);
	}
	if (RelicTabButton)
	{
		RelicTabButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionWidget::HandleRelicTabClicked);
	}
	if (AchievementTabButton)
	{
		AchievementTabButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionWidget::HandleAchievementTabClicked);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionWidget::HandleCloseClicked);
	}
	if (SearchTextBox)
	{
		SearchTextBox->OnTextChanged.AddUniqueDynamic(this, &UPBCollectionWidget::HandleSearchTextChanged);
	}
	if (AttackTypeComboBox)
	{
		AttackTypeComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UPBCollectionWidget::HandleAttackTypeFilterChanged);
	}
	if (RoleComboBox)
	{
		RoleComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UPBCollectionWidget::HandleRoleFilterChanged);
	}
	if (AttributeComboBox)
	{
		AttributeComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UPBCollectionWidget::HandleAttributeFilterChanged);
	}
	if (StarGradeComboBox)
	{
		StarGradeComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UPBCollectionWidget::HandleStarGradeFilterChanged);
	}
	if (SortModeComboBox)
	{
		SortModeComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UPBCollectionWidget::HandleSortModeChanged);
	}
}

bool UPBCollectionWidget::ValidateRequiredWidgetBindings() const
{
	bool bAllWidgetsBound = true;
	const auto CheckBinding = [this, &bAllWidgetsBound](const UObject* Widget, const TCHAR* WidgetName)
	{
		if (!IsValid(Widget))
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("%s: Widget Blueprint에 필수 위젯 '%s'가 없거나 이름/타입이 일치하지 않습니다."),
				*GetName(),
				WidgetName);
			bAllWidgetsBound = false;
		}
	};

	CheckBinding(EntryGridPanel, TEXT("EntryGridPanel"));
	CheckBinding(SearchTextBox, TEXT("SearchTextBox"));
	CheckBinding(AttackTypeComboBox, TEXT("AttackTypeComboBox"));
	CheckBinding(RoleComboBox, TEXT("RoleComboBox"));
	CheckBinding(AttributeComboBox, TEXT("AttributeComboBox"));
	CheckBinding(StarGradeComboBox, TEXT("StarGradeComboBox"));
	CheckBinding(SortModeComboBox, TEXT("SortModeComboBox"));
	CheckBinding(DetailNameText, TEXT("DetailNameText"));
	CheckBinding(DetailMetaText, TEXT("DetailMetaText"));
	CheckBinding(DetailDescriptionText, TEXT("DetailDescriptionText"));
	CheckBinding(DetailUnlockText, TEXT("DetailUnlockText"));
	CheckBinding(DetailRecordText, TEXT("DetailRecordText"));
	CheckBinding(DetailAccentBorder, TEXT("DetailAccentBorder"));
	CheckBinding(AllTabButton, TEXT("AllTabButton"));
	CheckBinding(BallTabButton, TEXT("BallTabButton"));
	CheckBinding(BumperTabButton, TEXT("BumperTabButton"));
	CheckBinding(BossTabButton, TEXT("BossTabButton"));
	CheckBinding(AchievementTabButton, TEXT("AchievementTabButton"));
	CheckBinding(CloseButton, TEXT("CloseButton"));

	return bAllWidgetsBound;
}

void UPBCollectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (GetClass() == StaticClass())
	{
		return;
	}

	if (!EntryWidgetClass)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("%s: EntryWidgetClass가 비어 있습니다. 도감 항목 Widget Blueprint를 지정해야 합니다."),
			*GetName());
	}

	CollectionSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UPBCollectionSubsystem>()
		: nullptr;

	if (CollectionSubsystem)
	{
		CollectionSubsystem->OnCollectionEntryChanged.AddUniqueDynamic(
			this,
			&UPBCollectionWidget::HandleCollectionEntryChanged);
		CollectionSubsystem->OnCollectionDataReady.AddUniqueDynamic(
			this,
			&UPBCollectionWidget::HandleCollectionDataReady);
	}

	PopulateFilterOptions();
	SetCategory(CurrentCategory);
}

void UPBCollectionWidget::NativeDestruct()
{
	if (CollectionSubsystem)
	{
		CollectionSubsystem->OnCollectionEntryChanged.RemoveDynamic(
			this,
			&UPBCollectionWidget::HandleCollectionEntryChanged);
		CollectionSubsystem->OnCollectionDataReady.RemoveDynamic(
			this,
			&UPBCollectionWidget::HandleCollectionDataReady);
	}

	Super::NativeDestruct();
}

void UPBCollectionWidget::OnPushed_Implementation()
{
	Super::OnPushed_Implementation();

	if (RedirectLegacyNativeWidget())
	{
		return;
	}

	ApplyCollectionInputMode(true);
}

void UPBCollectionWidget::OnPopped_Implementation()
{
	ApplyCollectionInputMode(false);

	Super::OnPopped_Implementation();
}

void UPBCollectionWidget::RefreshCollection()
{
	RefreshEntryList();
	RefreshDetail();
}

bool UPBCollectionWidget::RedirectLegacyNativeWidget()
{
	if (GetClass() != StaticClass())
	{
		return false;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UPBUIManagerSubsystem* UIManagerSubsystem = GameInstance
		? GameInstance->GetSubsystem<UPBUIManagerSubsystem>()
		: nullptr;
	if (!IsValid(UIManagerSubsystem))
	{
		UE_LOG(LogTemp, Error, TEXT("%s: 도감 WBP 전환에 필요한 UIManager를 찾지 못했습니다."), *GetName());
		return true;
	}

	const TSubclassOf<UPBCollectionWidget> BlueprintWidgetClass =
		LegacyCollectionWidgetBlueprintPath.TryLoadClass<UPBCollectionWidget>();
	if (!BlueprintWidgetClass)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("%s: 도감 Widget Blueprint를 불러오지 못했습니다. Path=%s"),
			*GetName(),
			*LegacyCollectionWidgetBlueprintPath.ToString());
		UIManagerSubsystem->CompletePopWidget(this);
		return true;
	}

	if (!UIManagerSubsystem->CompletePopWidget(this))
	{
		UE_LOG(LogTemp, Error, TEXT("%s: 기존 C++ 도감 위젯을 UI 스택에서 제거하지 못했습니다."), *GetName());
		return true;
	}

	UPBUserWidget* BlueprintWidget = UIManagerSubsystem->PushWidget(BlueprintWidgetClass, 0);
	if (!IsValid(BlueprintWidget))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("%s: 도감 Widget Blueprint를 UI 스택에 추가하지 못했습니다."),
			*GetName());
	}
	else
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[Collection] 기존 C++ 도감 참조를 Widget Blueprint로 전환했습니다. Widget=%s"),
			*GetNameSafe(BlueprintWidget));
	}

	return true;
}

void UPBCollectionWidget::PopulateFilterOptions()
{
	if (!CollectionSubsystem)
	{
		return;
	}

	bIsPopulatingFilterOptions = true;
	if (SearchTextBox)
	{
		SearchTextBox->SetText(FText::GetEmpty());
	}

	AttackTypeFilterIds = CollectionSubsystem->GetAvailableMetadataIds(EPBCollectionFilterField::AttackType);
	RoleFilterIds = CollectionSubsystem->GetAvailableMetadataIds(EPBCollectionFilterField::Role);
	AttributeFilterIds = CollectionSubsystem->GetAvailableMetadataIds(EPBCollectionFilterField::Attribute);

	PopulateMetadataComboBox(
		AttackTypeComboBox,
		AttackTypeFilterIds,
		NSLOCTEXT("PBCollection", "AttackTypeAll", "공격 전체"));
	PopulateMetadataComboBox(
		RoleComboBox,
		RoleFilterIds,
		NSLOCTEXT("PBCollection", "RoleAll", "역할 전체"));
	PopulateMetadataComboBox(
		AttributeComboBox,
		AttributeFilterIds,
		NSLOCTEXT("PBCollection", "AttributeAll", "속성 전체"));

	StarGradeFilterMap.Reset();
	if (StarGradeComboBox)
	{
		StarGradeComboBox->ClearOptions();
		const FString AllStarGradeText = NSLOCTEXT("PBCollection", "StarGradeAll", "성급 전체").ToString();
		StarGradeComboBox->AddOption(AllStarGradeText);
		StarGradeFilterMap.Add(AllStarGradeText, 0);

		for (const int32 StarGrade : CollectionSubsystem->GetAvailableStarGrades())
		{
			const FString StarGradeText = FString::Printf(TEXT("%d성"), StarGrade);
			StarGradeComboBox->AddOption(StarGradeText);
			StarGradeFilterMap.Add(StarGradeText, StarGrade);
		}
		StarGradeComboBox->SetSelectedOption(AllStarGradeText);
	}

	SortModeMap.Reset();
	if (SortModeComboBox)
	{
		SortModeComboBox->ClearOptions();

		const TArray<TPair<FString, EPBCollectionSortMode>> SortOptions = {
			TPair<FString, EPBCollectionSortMode>(
				NSLOCTEXT("PBCollection", "SortDefault", "기본순").ToString(),
				EPBCollectionSortMode::SortOrder),
			TPair<FString, EPBCollectionSortMode>(
				NSLOCTEXT("PBCollection", "SortName", "이름순").ToString(),
				EPBCollectionSortMode::NameAsc),
			TPair<FString, EPBCollectionSortMode>(
				NSLOCTEXT("PBCollection", "SortStarDesc", "성급 높은순").ToString(),
				EPBCollectionSortMode::StarGradeDesc),
			TPair<FString, EPBCollectionSortMode>(
				NSLOCTEXT("PBCollection", "SortStarAsc", "성급 낮은순").ToString(),
				EPBCollectionSortMode::StarGradeAsc),
			TPair<FString, EPBCollectionSortMode>(
				NSLOCTEXT("PBCollection", "SortState", "진행순").ToString(),
				EPBCollectionSortMode::StateDesc)
		};

		for (const TPair<FString, EPBCollectionSortMode>& SortOption : SortOptions)
		{
			SortModeComboBox->AddOption(SortOption.Key);
			SortModeMap.Add(SortOption.Key, SortOption.Value);
		}
		SortModeComboBox->SetSelectedOption(SortOptions[0].Key);
	}

	CurrentQuery = FPBCollectionQuery();
	CurrentQuery.Category = CurrentCategory;
	bIsPopulatingFilterOptions = false;
}

void UPBCollectionWidget::PopulateMetadataComboBox(
	UComboBoxString* ComboBox,
	const TArray<FName>& OptionIds,
	const FText& AllOptionText) const
{
	if (!ComboBox)
	{
		return;
	}

	const FString AllOptionString = AllOptionText.ToString();
	ComboBox->ClearOptions();
	ComboBox->AddOption(AllOptionString);

	for (const FName& OptionId : OptionIds)
	{
		ComboBox->AddOption(UPBCollectionSubsystem::GetMetadataDisplayText(OptionId).ToString());
	}

	ComboBox->SetSelectedOption(AllOptionString);
}

FName UPBCollectionWidget::ResolveMetadataSelection(const FString& SelectedItem, const TArray<FName>& OptionIds) const
{
	for (const FName& OptionId : OptionIds)
	{
		if (UPBCollectionSubsystem::GetMetadataDisplayText(OptionId).ToString() == SelectedItem)
		{
			return OptionId;
		}
	}

	return NAME_None;
}

void UPBCollectionWidget::RefreshCollectionByFilterChange()
{
	if (bIsPopulatingFilterOptions)
	{
		return;
	}

	RefreshCollection();
}

void UPBCollectionWidget::RefreshEntryList()
{
	if (!CollectionSubsystem || !EntryGridPanel || !EntryWidgetClass)
	{
		return;
	}

	EntryGridPanel->ClearChildren();

	CurrentQuery.Category = CurrentCategory;
	const TArray<FPBCollectionDisplayData> Entries = CollectionSubsystem->GetDisplayEntriesByQuery(CurrentQuery);
	bool bSelectedEntryVisible = false;

	for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
	{
		const FPBCollectionDisplayData& EntryData = Entries[EntryIndex];
		UPBCollectionEntryWidget* EntryWidget = CreateWidget<UPBCollectionEntryWidget>(
			ResolvePlayerController(),
			EntryWidgetClass);
		if (!EntryWidget)
		{
			continue;
		}

		EntryWidget->SetDisplayData(EntryData);
		EntryWidget->OnEntryClicked.AddUniqueDynamic(this, &UPBCollectionWidget::HandleEntryClicked);

		const int32 Row = EntryIndex / 3;
		const int32 Column = EntryIndex % 3;
		UUniformGridSlot* GridSlot = EntryGridPanel->AddChildToUniformGrid(EntryWidget, Row, Column);
		GridSlot->SetHorizontalAlignment(HAlign_Fill);
		GridSlot->SetVerticalAlignment(VAlign_Fill);

		if (EntryData.CollectionId == SelectedCollectionId)
		{
			bSelectedEntryVisible = true;
		}
	}

	if (!bSelectedEntryVisible)
	{
		SelectedCollectionId = Entries.Num() > 0 ? Entries[0].CollectionId : NAME_None;
	}
}

void UPBCollectionWidget::RefreshDetail()
{
	FPBCollectionDisplayData DisplayData;
	const bool bHasSelection = CollectionSubsystem
		&& SelectedCollectionId != NAME_None
		&& CollectionSubsystem->GetDisplayEntry(SelectedCollectionId, DisplayData);

	if (!bHasSelection)
	{
		const bool bIsLoading = CollectionSubsystem && !CollectionSubsystem->IsDataReady();
		if (DetailNameText)
		{
			DetailNameText->SetText(bIsLoading
				? NSLOCTEXT("PBCollection", "LoadingName", "도감 불러오는 중")
				: NSLOCTEXT("PBCollection", "NoSelectionName", "도감"));
		}
		if (DetailMetaText)
		{
			DetailMetaText->SetText(bIsLoading
				? NSLOCTEXT("PBCollection", "LoadingMeta", "잠시만 기다려 주세요.")
				: NSLOCTEXT("PBCollection", "NoSelectionMeta", "항목 없음"));
		}
		if (DetailDescriptionText)
		{
			DetailDescriptionText->SetText(FText::GetEmpty());
		}
		if (DetailUnlockText)
		{
			DetailUnlockText->SetText(FText::GetEmpty());
		}
		if (DetailRecordText)
		{
			DetailRecordText->SetText(FText::GetEmpty());
		}
		return;
	}

	if (DetailNameText)
	{
		DetailNameText->SetText(DisplayData.DisplayName);
	}
	if (DetailMetaText)
	{
		DetailMetaText->SetText(FText::Format(
			NSLOCTEXT("PBCollection", "DetailMetaFormat", "{0} · {1} · {2} · {3} · {4} · {5}성"),
			DisplayData.CategoryText,
			DisplayData.StateText,
			DisplayData.AttackTypeText,
			DisplayData.RoleText,
			DisplayData.AttributeText,
			FText::AsNumber(DisplayData.StarGrade)));
	}
	if (DetailDescriptionText)
	{
		DetailDescriptionText->SetText(DisplayData.DetailDescription);
	}
	if (DetailUnlockText)
	{
		DetailUnlockText->SetText(FText::Format(
			NSLOCTEXT("PBCollection", "UnlockFormat", "해금 조건: {0}"),
			DisplayData.UnlockConditionText));
	}
	if (DetailRecordText)
	{
		DetailRecordText->SetText(DisplayData.bCanShowFullData
			? DisplayData.RecordText
			: NSLOCTEXT("PBCollection", "LockedRecordText", "기록은 해금 이후 표시됩니다."));
	}
	if (DetailAccentBorder)
	{
		DetailAccentBorder->SetBrushColor(DisplayData.State == EPBCollectionState::Locked
			? FLinearColor(0.22f, 0.22f, 0.22f, 1.0f)
			: DisplayData.AccentColor);
	}
}

void UPBCollectionWidget::SetCategory(EPBCollectionCategory NewCategory)
{
	CurrentCategory = NewCategory;

	BP_OnCollectionCategoryChanged(NewCategory);
	RefreshCollection();
}

void UPBCollectionWidget::SelectEntry(FName CollectionId)
{
	SelectedCollectionId = CollectionId;
	RefreshDetail();
	if (CollectionSubsystem)
	{
		CollectionSubsystem->MarkEntryAsSeen(CollectionId);
	}
}

APlayerController* UPBCollectionWidget::ResolvePlayerController() const
{
	if (APlayerController* OwningPlayer = GetOwningPlayer())
	{
		return OwningPlayer;
	}

	if (UWorld* World = GetWorld())
	{
		return World->GetFirstPlayerController();
	}

	return nullptr;
}

void UPBCollectionWidget::ApplyCollectionInputMode(const bool bEnableUI) const
{
	if (!bEnableUI)
	{
		if (const UGameInstance* GameInstance = GetGameInstance())
		{
			const UPBUIManagerSubsystem* UIManagerSubsystem =
				GameInstance->GetSubsystem<UPBUIManagerSubsystem>();
			if (IsValid(UIManagerSubsystem) && IsValid(UIManagerSubsystem->GetTopWidget()))
			{
				return;
			}
		}
	}

	APlayerController* PlayerController = ResolvePlayerController();
	if (!PlayerController)
	{
		return;
	}

	PlayerController->bShowMouseCursor = bEnableUI;

	if (bEnableUI)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputMode);
	}
	else
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
}

void UPBCollectionWidget::HandleAllTabClicked()
{
	SetCategory(EPBCollectionCategory::All);
}

void UPBCollectionWidget::HandleBallTabClicked()
{
	SetCategory(EPBCollectionCategory::Ball);
}

void UPBCollectionWidget::HandleBumperTabClicked()
{
	SetCategory(EPBCollectionCategory::Bumper);
}

void UPBCollectionWidget::HandleBossTabClicked()
{
	SetCategory(EPBCollectionCategory::Boss);
}

void UPBCollectionWidget::HandleRelicTabClicked()
{
	SetCategory(EPBCollectionCategory::Relic);
}

void UPBCollectionWidget::HandleAchievementTabClicked()
{
	SetCategory(EPBCollectionCategory::Achievement);
}

void UPBCollectionWidget::HandleCloseClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPBUIManagerSubsystem* UIManagerSubsystem = GameInstance->GetSubsystem<UPBUIManagerSubsystem>())
		{
			if (UIManagerSubsystem->GetTopWidget() == this)
			{
				if (IsPopRequested() || UIManagerSubsystem->RequestPopWidget())
				{
					return;
				}
			}
		}
	}

	// 스택 상태가 예상과 다를 때도 UIManager가 제거 가능 여부를 최종 판단하도록 요청합니다.
	CompletePop();
}

void UPBCollectionWidget::HandleSearchTextChanged(const FText& Text)
{
	CurrentQuery.SearchText = Text.ToString();
	RefreshCollectionByFilterChange();
}

void UPBCollectionWidget::HandleAttackTypeFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	CurrentQuery.AttackTypeId = ResolveMetadataSelection(SelectedItem, AttackTypeFilterIds);
	RefreshCollectionByFilterChange();
}

void UPBCollectionWidget::HandleRoleFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	CurrentQuery.RoleId = ResolveMetadataSelection(SelectedItem, RoleFilterIds);
	RefreshCollectionByFilterChange();
}

void UPBCollectionWidget::HandleAttributeFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	CurrentQuery.AttributeId = ResolveMetadataSelection(SelectedItem, AttributeFilterIds);
	RefreshCollectionByFilterChange();
}

void UPBCollectionWidget::HandleStarGradeFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (const int32* StarGrade = StarGradeFilterMap.Find(SelectedItem))
	{
		CurrentQuery.StarGrade = *StarGrade;
	}
	else
	{
		CurrentQuery.StarGrade = 0;
	}

	RefreshCollectionByFilterChange();
}

void UPBCollectionWidget::HandleSortModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (const EPBCollectionSortMode* SortMode = SortModeMap.Find(SelectedItem))
	{
		CurrentQuery.SortMode = *SortMode;
	}
	else
	{
		CurrentQuery.SortMode = EPBCollectionSortMode::SortOrder;
	}

	RefreshCollectionByFilterChange();
}

void UPBCollectionWidget::HandleEntryClicked(FName CollectionId)
{
	SelectEntry(CollectionId);
}

void UPBCollectionWidget::HandleCollectionEntryChanged(FName CollectionId)
{
	RefreshCollection();
}

void UPBCollectionWidget::HandleCollectionDataReady(bool bIsReady)
{
	if (!bIsReady)
	{
		return;
	}

	PopulateFilterOptions();
	SetCategory(CurrentCategory);
}
