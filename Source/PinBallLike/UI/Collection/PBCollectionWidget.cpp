#include "PBCollectionWidget.h"

#include "PBCollectionEntryWidget.h"
#include "PinBallLike/Subsystem/PBCollectionSubsystem.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace
{
constexpr float TabLabelWidth = 96.0f;
constexpr float DemoButtonLabelWidth = 82.0f;
constexpr float DetailPanelWidth = 440.0f;

FSlateBrush MakeColorBrush(const FLinearColor& Color, const FVector2D& ImageSize)
{
	FSlateBrush Brush;
	Brush.ImageSize = ImageSize;
	Brush.DrawAs = ESlateBrushDrawType::Box;
	Brush.TintColor = FSlateColor(Color);
	return Brush;
}

FButtonStyle MakeCollectionButtonStyle(
	const FLinearColor& NormalColor,
	const FLinearColor& HoveredColor,
	const FLinearColor& PressedColor,
	const FVector2D& ImageSize)
{
	FButtonStyle Style;
	Style.SetNormal(MakeColorBrush(NormalColor, ImageSize));
	Style.SetHovered(MakeColorBrush(HoveredColor, ImageSize));
	Style.SetPressed(MakeColorBrush(PressedColor, ImageSize));
	Style.SetNormalPadding(FMargin(12.0f, 8.0f));
	Style.SetPressedPadding(FMargin(12.0f, 10.0f, 12.0f, 6.0f));
	return Style;
}

FButtonStyle MakeDefaultButtonStyle()
{
	return MakeCollectionButtonStyle(
		FLinearColor(0.08f, 0.16f, 0.20f, 1.0f),
		FLinearColor(0.12f, 0.28f, 0.34f, 1.0f),
		FLinearColor(0.05f, 0.11f, 0.14f, 1.0f),
		FVector2D(128.0f, 36.0f));
}

FButtonStyle MakeTabButtonStyle(bool bActive)
{
	return MakeCollectionButtonStyle(
		bActive
			? FLinearColor(0.10f, 0.36f, 0.42f, 1.0f)
			: FLinearColor(0.07f, 0.14f, 0.18f, 1.0f),
		FLinearColor(0.14f, 0.30f, 0.36f, 1.0f),
		FLinearColor(0.05f, 0.11f, 0.14f, 1.0f),
		FVector2D(128.0f, 36.0f));
}

FLinearColor GetAccentColor(EPBCollectionCategory Category, EPBCollectionState State)
{
	if (State == EPBCollectionState::Locked)
	{
		return FLinearColor(0.28f, 0.28f, 0.32f, 1.0f);
	}

	switch (Category)
	{
	case EPBCollectionCategory::Ball:
		return FLinearColor(0.88f, 0.22f, 0.18f, 1.0f);
	case EPBCollectionCategory::Bumper:
		return FLinearColor(0.12f, 0.72f, 0.86f, 1.0f);
	case EPBCollectionCategory::Boss:
		return FLinearColor(0.72f, 0.24f, 0.86f, 1.0f);
	case EPBCollectionCategory::Relic:
		return FLinearColor(0.92f, 0.66f, 0.22f, 1.0f);
	case EPBCollectionCategory::Achievement:
		return FLinearColor(0.34f, 0.72f, 0.42f, 1.0f);
	default:
		return FLinearColor(0.12f, 0.72f, 0.86f, 1.0f);
	}
}
}

void UPBCollectionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BuildDefaultWidgetTree();

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
	if (DiscoverButton)
	{
		DiscoverButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionWidget::HandleDiscoverClicked);
	}
	if (UnlockButton)
	{
		UnlockButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionWidget::HandleUnlockClicked);
	}
	if (CompleteButton)
	{
		CompleteButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionWidget::HandleCompleteClicked);
	}
	if (ResetButton)
	{
		ResetButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionWidget::HandleResetClicked);
	}
	if (SearchTextBox)
	{
		SearchTextBox->OnTextChanged.AddDynamic(this, &UPBCollectionWidget::HandleSearchTextChanged);
	}
	if (AttackTypeComboBox)
	{
		AttackTypeComboBox->OnSelectionChanged.AddDynamic(this, &UPBCollectionWidget::HandleAttackTypeFilterChanged);
	}
	if (RoleComboBox)
	{
		RoleComboBox->OnSelectionChanged.AddDynamic(this, &UPBCollectionWidget::HandleRoleFilterChanged);
	}
	if (AttributeComboBox)
	{
		AttributeComboBox->OnSelectionChanged.AddDynamic(this, &UPBCollectionWidget::HandleAttributeFilterChanged);
	}
	if (StarGradeComboBox)
	{
		StarGradeComboBox->OnSelectionChanged.AddDynamic(this, &UPBCollectionWidget::HandleStarGradeFilterChanged);
	}
	if (SortModeComboBox)
	{
		SortModeComboBox->OnSelectionChanged.AddDynamic(this, &UPBCollectionWidget::HandleSortModeChanged);
	}
}

void UPBCollectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!EntryWidgetClass)
	{
		EntryWidgetClass = UPBCollectionEntryWidget::StaticClass();
	}

	CollectionSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UPBCollectionSubsystem>()
		: nullptr;

	if (CollectionSubsystem)
	{
		CollectionSubsystem->OnCollectionEntryChanged.AddUniqueDynamic(
			this,
			&UPBCollectionWidget::HandleCollectionEntryChanged);
	}

	PopulateFilterOptions();
	RefreshCollection();
}

void UPBCollectionWidget::NativeDestruct()
{
	if (CollectionSubsystem)
	{
		CollectionSubsystem->OnCollectionEntryChanged.RemoveDynamic(
			this,
			&UPBCollectionWidget::HandleCollectionEntryChanged);
	}

	Super::NativeDestruct();
}

void UPBCollectionWidget::RefreshCollection()
{
	RefreshEntryList();
	RefreshDetail();
}

void UPBCollectionWidget::BuildDefaultWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RootBorder"));
	RootBorder->SetBrush(MakeColorBrush(FLinearColor(0.015f, 0.018f, 0.024f, 1.0f), FVector2D(1920.0f, 1080.0f)));
	RootBorder->SetPadding(FMargin(26.0f, 24.0f));
	WidgetTree->RootWidget = RootBorder;

	UBorder* WindowFrameBorder = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(),
		TEXT("WindowFrameBorder"));
	WindowFrameBorder->SetBrush(MakeColorBrush(FLinearColor(0.025f, 0.045f, 0.055f, 0.96f), FVector2D(1606.0f, 979.0f)));
	WindowFrameBorder->SetPadding(FMargin(34.0f, 34.0f, 34.0f, 34.0f));
	RootBorder->SetContent(WindowFrameBorder);

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RootBox"));
	WindowFrameBorder->SetContent(RootBox);

	BuildHeader(RootBox);
	BuildBody(RootBox);
}

void UPBCollectionWidget::BuildHeader(UVerticalBox* RootBox)
{
	UHorizontalBox* HeaderBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HeaderBox"));
	RootBox->AddChildToVerticalBox(HeaderBox);
	if (UVerticalBoxSlot* HeaderSlot = Cast<UVerticalBoxSlot>(HeaderBox->Slot))
	{
		HeaderSlot->SetPadding(FMargin(36.0f, 10.0f, 36.0f, 12.0f));
		HeaderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	AllTabButton = CreateTextButton(WidgetTree, TEXT("AllTabButton"), NSLOCTEXT("PBCollection", "AllTab", "전체"));
	BallTabButton = CreateTextButton(WidgetTree, TEXT("BallTabButton"), NSLOCTEXT("PBCollection", "BallTab", "Ball"));
	BumperTabButton = CreateTextButton(WidgetTree, TEXT("BumperTabButton"), NSLOCTEXT("PBCollection", "BumperTab", "Bumper"));
	BossTabButton = CreateTextButton(WidgetTree, TEXT("BossTabButton"), NSLOCTEXT("PBCollection", "BossTab", "Boss"));
	RelicTabButton = CreateTextButton(WidgetTree, TEXT("RelicTabButton"), NSLOCTEXT("PBCollection", "RelicTab", "Relic"));
	AchievementTabButton = CreateTextButton(WidgetTree, TEXT("AchievementTabButton"), NSLOCTEXT("PBCollection", "AchievementTab", "업적"));

	const TArray<UButton*> TabButtons = {
		AllTabButton,
		BallTabButton,
		BumperTabButton,
		BossTabButton,
		RelicTabButton,
		AchievementTabButton
	};

	for (UButton* TabButton : TabButtons)
	{
		TabButton->SetStyle(MakeTabButtonStyle(TabButton == AllTabButton));
		HeaderBox->AddChildToHorizontalBox(TabButton);
		if (UHorizontalBoxSlot* TabSlot = Cast<UHorizontalBoxSlot>(TabButton->Slot))
		{
			TabSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
			TabSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		}
	}

	UTextBlock* SpacerText = CreateText(WidgetTree, TEXT("HeaderSpacerText"), 1, FLinearColor::Transparent);
	HeaderBox->AddChildToHorizontalBox(SpacerText);
	if (UHorizontalBoxSlot* SpacerSlot = Cast<UHorizontalBoxSlot>(SpacerText->Slot))
	{
		SpacerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UButton* CloseButton = CreateTextButton(WidgetTree, TEXT("CloseButton"), NSLOCTEXT("PBCollection", "CloseButton", "닫기"));
	HeaderBox->AddChildToHorizontalBox(CloseButton);
	CloseButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionWidget::HandleCloseClicked);
}

void UPBCollectionWidget::BuildBody(UVerticalBox* RootBox)
{
	UHorizontalBox* BodyBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BodyBox"));
	RootBox->AddChildToVerticalBox(BodyBox);
	if (UVerticalBoxSlot* BodySlot = Cast<UVerticalBoxSlot>(BodyBox->Slot))
	{
		BodySlot->SetPadding(FMargin(36.0f, 0.0f, 36.0f, 28.0f));
		BodySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UBorder* ListBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ListBorder"));
	ListBorder->SetBrush(MakeColorBrush(FLinearColor(0.045f, 0.065f, 0.078f, 0.94f), FVector2D(520.0f, 720.0f)));
	ListBorder->SetPadding(FMargin(18.0f, 18.0f, 18.0f, 18.0f));
	BodyBox->AddChildToHorizontalBox(ListBorder);
	if (UHorizontalBoxSlot* ListSlot = Cast<UHorizontalBoxSlot>(ListBorder->Slot))
	{
		ListSlot->SetPadding(FMargin(0.0f, 0.0f, 16.0f, 0.0f));
		ListSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UVerticalBox* ListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ListBox"));
	ListBorder->SetContent(ListBox);

	BuildFilterPanel(ListBox);

	UScrollBox* EntryScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("EntryScrollBox"));
	ListBox->AddChildToVerticalBox(EntryScrollBox);
	if (UVerticalBoxSlot* EntryScrollSlot = Cast<UVerticalBoxSlot>(EntryScrollBox->Slot))
	{
		EntryScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		EntryScrollSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	EntryGridPanel = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("EntryGridPanel"));
	EntryGridPanel->SetMinDesiredSlotWidth(264.0f);
	EntryGridPanel->SetMinDesiredSlotHeight(264.0f);
	EntryGridPanel->SetSlotPadding(FMargin(8.0f));
	EntryScrollBox->AddChild(EntryGridPanel);

	BuildDetailPanel(BodyBox);
}

void UPBCollectionWidget::BuildFilterPanel(UVerticalBox* ListBox)
{
	UVerticalBox* FilterBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("FilterBox"));
	ListBox->AddChildToVerticalBox(FilterBox);
	if (UVerticalBoxSlot* FilterSlot = Cast<UVerticalBoxSlot>(FilterBox->Slot))
	{
		FilterSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		FilterSlot->SetPadding(FMargin(0.0f));
	}

	auto AddSizedControl = [this](UHorizontalBox* RowBox, UWidget* Control, float Width)
	{
		USizeBox* ControlSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		ControlSizeBox->SetWidthOverride(Width);
		ControlSizeBox->SetHeightOverride(34.0f);
		ControlSizeBox->AddChild(Control);
		RowBox->AddChildToHorizontalBox(ControlSizeBox);
		if (UHorizontalBoxSlot* ControlSlot = Cast<UHorizontalBoxSlot>(ControlSizeBox->Slot))
		{
			ControlSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 6.0f));
			ControlSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		}
	};

	UHorizontalBox* SearchRowBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("SearchRowBox"));
	FilterBox->AddChildToVerticalBox(SearchRowBox);

	SearchTextBox = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("SearchTextBox"));
	SearchTextBox->SetHintText(NSLOCTEXT("PBCollection", "SearchHint", "검색"));
	AddSizedControl(SearchRowBox, SearchTextBox, 280.0f);

	SortModeComboBox = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("SortModeComboBox"));
	AddSizedControl(SearchRowBox, SortModeComboBox, 156.0f);

	UHorizontalBox* FilterRowBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("FilterRowBox"));
	FilterBox->AddChildToVerticalBox(FilterRowBox);

	AttackTypeComboBox = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("AttackTypeComboBox"));
	AddSizedControl(FilterRowBox, AttackTypeComboBox, 136.0f);

	RoleComboBox = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("RoleComboBox"));
	AddSizedControl(FilterRowBox, RoleComboBox, 136.0f);

	AttributeComboBox = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("AttributeComboBox"));
	AddSizedControl(FilterRowBox, AttributeComboBox, 136.0f);

	StarGradeComboBox = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("StarGradeComboBox"));
	AddSizedControl(FilterRowBox, StarGradeComboBox, 136.0f);
}

void UPBCollectionWidget::BuildDetailPanel(UHorizontalBox* BodyBox)
{
	USizeBox* DetailSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("DetailSizeBox"));
	DetailSizeBox->SetWidthOverride(DetailPanelWidth);
	BodyBox->AddChildToHorizontalBox(DetailSizeBox);
	if (UHorizontalBoxSlot* DetailSlot = Cast<UHorizontalBoxSlot>(DetailSizeBox->Slot))
	{
		DetailSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	UBorder* DetailBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DetailBorder"));
	DetailBorder->SetBrush(MakeColorBrush(FLinearColor(0.04f, 0.055f, 0.065f, 0.96f), FVector2D(DetailPanelWidth, 720.0f)));
	DetailBorder->SetPadding(FMargin(24.0f, 24.0f, 24.0f, 20.0f));
	DetailSizeBox->AddChild(DetailBorder);

	UScrollBox* DetailScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("DetailScrollBox"));
	DetailBorder->SetContent(DetailScrollBox);

	UVerticalBox* DetailBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DetailBox"));
	DetailScrollBox->AddChild(DetailBox);

	DetailAccentBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DetailAccentBorder"));
	DetailAccentBorder->SetBrush(MakeColorBrush(
		GetAccentColor(EPBCollectionCategory::Bumper, EPBCollectionState::Unlocked),
		FVector2D(DetailPanelWidth - 128.0f, 16.0f)));
	DetailBox->AddChildToVerticalBox(DetailAccentBorder);
	if (UVerticalBoxSlot* AccentSlot = Cast<UVerticalBoxSlot>(DetailAccentBorder->Slot))
	{
		AccentSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 14.0f));
		AccentSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	DetailNameText = CreateText(WidgetTree, TEXT("DetailNameText"), 22, FLinearColor(0.96f, 0.97f, 0.98f, 1.0f));
	DetailNameText->SetAutoWrapText(true);
	DetailBox->AddChildToVerticalBox(DetailNameText);
	if (UVerticalBoxSlot* NameSlot = Cast<UVerticalBoxSlot>(DetailNameText->Slot))
	{
		NameSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
	}

	DetailMetaText = CreateText(WidgetTree, TEXT("DetailMetaText"), 14, FLinearColor(0.70f, 0.75f, 0.80f, 1.0f));
	DetailBox->AddChildToVerticalBox(DetailMetaText);
	if (UVerticalBoxSlot* MetaSlot = Cast<UVerticalBoxSlot>(DetailMetaText->Slot))
	{
		MetaSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 16.0f));
	}

	DetailDescriptionText = CreateText(WidgetTree, TEXT("DetailDescriptionText"), 15, FLinearColor(0.88f, 0.90f, 0.92f, 1.0f));
	DetailDescriptionText->SetAutoWrapText(true);
	DetailBox->AddChildToVerticalBox(DetailDescriptionText);
	if (UVerticalBoxSlot* DescriptionSlot = Cast<UVerticalBoxSlot>(DetailDescriptionText->Slot))
	{
		DescriptionSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
	}

	DetailUnlockText = CreateText(WidgetTree, TEXT("DetailUnlockText"), 13, FLinearColor(0.72f, 0.78f, 0.82f, 1.0f));
	DetailUnlockText->SetAutoWrapText(true);
	DetailBox->AddChildToVerticalBox(DetailUnlockText);
	if (UVerticalBoxSlot* UnlockSlot = Cast<UVerticalBoxSlot>(DetailUnlockText->Slot))
	{
		UnlockSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
	}

	DetailRecordText = CreateText(WidgetTree, TEXT("DetailRecordText"), 13, FLinearColor(0.76f, 0.80f, 0.84f, 1.0f));
	DetailRecordText->SetAutoWrapText(true);
	DetailBox->AddChildToVerticalBox(DetailRecordText);
	if (UVerticalBoxSlot* RecordSlot = Cast<UVerticalBoxSlot>(DetailRecordText->Slot))
	{
		RecordSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
		RecordSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	UUniformGridPanel* ButtonGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("DemoButtonGrid"));
	ButtonGrid->SetMinDesiredSlotWidth(146.0f);
	ButtonGrid->SetMinDesiredSlotHeight(42.0f);
	ButtonGrid->SetSlotPadding(FMargin(4.0f));
	DetailBox->AddChildToVerticalBox(ButtonGrid);
	if (UVerticalBoxSlot* ButtonGridSlot = Cast<UVerticalBoxSlot>(ButtonGrid->Slot))
	{
		ButtonGridSlot->SetPadding(FMargin(0.0f));
		ButtonGridSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	DiscoverButton = CreateTextButton(WidgetTree, TEXT("DiscoverButton"), NSLOCTEXT("PBCollection", "DiscoverButton", "발견"));
	UnlockButton = CreateTextButton(WidgetTree, TEXT("UnlockButton"), NSLOCTEXT("PBCollection", "UnlockButton", "해금"));
	CompleteButton = CreateTextButton(WidgetTree, TEXT("CompleteButton"), NSLOCTEXT("PBCollection", "CompleteButton", "완료"));
	ResetButton = CreateTextButton(WidgetTree, TEXT("ResetButton"), NSLOCTEXT("PBCollection", "ResetButton", "초기화"));

	const TArray<UButton*> DemoButtons = {
		DiscoverButton,
		UnlockButton,
		CompleteButton,
		ResetButton
	};

	for (int32 ButtonIndex = 0; ButtonIndex < DemoButtons.Num(); ++ButtonIndex)
	{
		UButton* DemoButton = DemoButtons[ButtonIndex];
		UUniformGridSlot* ButtonSlot = ButtonGrid->AddChildToUniformGrid(
			DemoButton,
			ButtonIndex / 2,
			ButtonIndex % 2);
		if (ButtonSlot)
		{
			ButtonSlot->SetHorizontalAlignment(HAlign_Fill);
			ButtonSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}
}

UButton* UPBCollectionWidget::CreateTextButton(UWidgetTree* InWidgetTree, FName WidgetName, const FText& Label)
{
	UButton* Button = InWidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), WidgetName);
	Button->SetStyle(MakeDefaultButtonStyle());

	UTextBlock* LabelText = CreateText(
		InWidgetTree,
		FName(*(WidgetName.ToString() + TEXT("_Label"))),
		14,
		FLinearColor(0.92f, 0.94f, 0.96f, 1.0f));
	LabelText->SetText(Label);
	LabelText->SetMinDesiredWidth(WidgetName.ToString().Contains(TEXT("Tab")) ? TabLabelWidth : DemoButtonLabelWidth);
	LabelText->SetJustification(ETextJustify::Center);
	Button->AddChild(LabelText);
	if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(LabelText->Slot))
	{
		ButtonSlot->SetPadding(FMargin(0.0f));
		ButtonSlot->SetHorizontalAlignment(HAlign_Center);
		ButtonSlot->SetVerticalAlignment(VAlign_Center);
	}
	return Button;
}

UTextBlock* UPBCollectionWidget::CreateText(
	UWidgetTree* InWidgetTree,
	FName WidgetName,
	int32 FontSize,
	const FLinearColor& Color)
{
	UTextBlock* TextBlock = InWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), WidgetName);
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetAutoWrapText(false);
	TextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
	TextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));

	FSlateFontInfo FontInfo = TextBlock->GetFont();
	FontInfo.Size = FontSize;
	TextBlock->SetFont(FontInfo);

	return TextBlock;
}

void UPBCollectionWidget::PopulateFilterOptions()
{
	if (!CollectionSubsystem)
	{
		return;
	}

	bIsPopulatingFilterOptions = true;

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
	if (!CollectionSubsystem || !EntryGridPanel)
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
		TSubclassOf<UPBCollectionEntryWidget> ActualEntryWidgetClass = EntryWidgetClass;
		if (!ActualEntryWidgetClass)
		{
			ActualEntryWidgetClass = UPBCollectionEntryWidget::StaticClass();
		}

		UPBCollectionEntryWidget* EntryWidget = CreateWidget<UPBCollectionEntryWidget>(
			GetOwningPlayer(),
			ActualEntryWidgetClass);
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
		if (DetailNameText)
		{
			DetailNameText->SetText(NSLOCTEXT("PBCollection", "NoSelectionName", "도감"));
		}
		if (DetailMetaText)
		{
			DetailMetaText->SetText(NSLOCTEXT("PBCollection", "NoSelectionMeta", "항목 없음"));
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
	if (AllTabButton)
	{
		AllTabButton->SetStyle(MakeTabButtonStyle(NewCategory == EPBCollectionCategory::All));
	}
	if (BallTabButton)
	{
		BallTabButton->SetStyle(MakeTabButtonStyle(NewCategory == EPBCollectionCategory::Ball));
	}
	if (BumperTabButton)
	{
		BumperTabButton->SetStyle(MakeTabButtonStyle(NewCategory == EPBCollectionCategory::Bumper));
	}
	if (BossTabButton)
	{
		BossTabButton->SetStyle(MakeTabButtonStyle(NewCategory == EPBCollectionCategory::Boss));
	}
	if (RelicTabButton)
	{
		RelicTabButton->SetStyle(MakeTabButtonStyle(NewCategory == EPBCollectionCategory::Relic));
	}
	if (AchievementTabButton)
	{
		AchievementTabButton->SetStyle(MakeTabButtonStyle(NewCategory == EPBCollectionCategory::Achievement));
	}
	RefreshCollection();
}

void UPBCollectionWidget::SelectEntry(FName CollectionId)
{
	SelectedCollectionId = CollectionId;
	RefreshDetail();
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
	RemoveFromParent();
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

void UPBCollectionWidget::HandleDiscoverClicked()
{
	if (CollectionSubsystem && SelectedCollectionId != NAME_None)
	{
		CollectionSubsystem->DiscoverEntry(SelectedCollectionId);
	}
}

void UPBCollectionWidget::HandleUnlockClicked()
{
	if (CollectionSubsystem && SelectedCollectionId != NAME_None)
	{
		CollectionSubsystem->UnlockEntry(SelectedCollectionId);
	}
}

void UPBCollectionWidget::HandleCompleteClicked()
{
	if (CollectionSubsystem && SelectedCollectionId != NAME_None)
	{
		CollectionSubsystem->CompleteEntry(SelectedCollectionId, TEXT("검사 볼"));
	}
}

void UPBCollectionWidget::HandleResetClicked()
{
	if (CollectionSubsystem)
	{
		CollectionSubsystem->ResetAllProgress();
	}
}

void UPBCollectionWidget::HandleEntryClicked(FName CollectionId)
{
	SelectEntry(CollectionId);
}

void UPBCollectionWidget::HandleCollectionEntryChanged(FName CollectionId)
{
	RefreshCollection();
}
