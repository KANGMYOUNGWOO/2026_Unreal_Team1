#include "PBGlobalToolbarWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "PinBallLike/DeveloperSettings/PBUISettings.h"
#include "PinBallLike/Relic/UI/PBRelicInventoryWidget.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Subsystem/PBUIManagerSubsystem.h"
#include "Styling/CoreStyle.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"

namespace
{
void ApplyToolbarButtonStyle(UButton* Button)
{
	if (!IsValid(Button))
	{
		return;
	}

	FButtonStyle Style = Button->GetStyle();
	Style.Normal.TintColor = FSlateColor(FLinearColor(0.10f, 0.16f, 0.19f, 0.96f));
	Style.Hovered.TintColor = FSlateColor(FLinearColor(0.18f, 0.31f, 0.36f, 1.0f));
	Style.Pressed.TintColor = FSlateColor(FLinearColor(0.07f, 0.12f, 0.15f, 1.0f));
	Style.SetNormalPadding(FMargin(0.0f));
	Style.SetPressedPadding(FMargin(1.0f));
	Button->SetStyle(Style);
	Button->SetBackgroundColor(FLinearColor::White);
}
}

UPBGlobalToolbarWidget::UPBGlobalToolbarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, RelicInventoryWidgetClass(FSoftObjectPath(
		TEXT("/Game/Blueprints/UI/Relic/WBP_PBRelicInventoryWidget.WBP_PBRelicInventoryWidget_C")))
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> DeckButtonIconFinder(
		TEXT("/Game/Resources/UI/Toolbar/T_UI_ToolbarDeck.T_UI_ToolbarDeck"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> OptionButtonIconFinder(
		TEXT("/Game/Resources/UI/Toolbar/T_UI_ToolbarOption.T_UI_ToolbarOption"));

	DeckButtonIcon = DeckButtonIconFinder.Object;
	OptionButtonIcon = OptionButtonIconFinder.Object;
}

void UPBGlobalToolbarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureAnchoredToolbarContent();

	if (DeckButton)
	{
		DeckButton->OnClicked.AddUniqueDynamic(
			this,
			&UPBGlobalToolbarWidget::HandleDeckButtonClicked);
	}

	UGameInstance* GameInstance = GetGameInstance();
	PlayerDataSubsystem = IsValid(GameInstance)
		? GameInstance->GetSubsystem<UPBPlayerDataSubsystem>()
		: nullptr;

	if (IsValid(PlayerDataSubsystem))
	{
		PlayerDataSubsystem->OnGoldChanged.AddUniqueDynamic(
			this,
			&UPBGlobalToolbarWidget::HandleGoldChanged);
		HandleGoldChanged(PlayerDataSubsystem->GetCurrentGold());
	}
}

void UPBGlobalToolbarWidget::NativeDestruct()
{
	if (DeckButton)
	{
		DeckButton->OnClicked.RemoveDynamic(
			this,
			&UPBGlobalToolbarWidget::HandleDeckButtonClicked);
	}

	if (IsValid(PlayerDataSubsystem))
	{
		PlayerDataSubsystem->OnGoldChanged.RemoveDynamic(
			this,
			&UPBGlobalToolbarWidget::HandleGoldChanged);
	}
	PlayerDataSubsystem = nullptr;

	Super::NativeDestruct();
}

void UPBGlobalToolbarWidget::HandleDeckButtonClicked()
{
	UGameInstance* GameInstance = GetGameInstance();
	UPBUIManagerSubsystem* UIManager = IsValid(GameInstance)
		? GameInstance->GetSubsystem<UPBUIManagerSubsystem>()
		: nullptr;
	if (UIManager)
	{
		UIManager->RequestGlobalDeckToggle();
	}
}

void UPBGlobalToolbarWidget::HandleGoldChanged(const int32 NewGold)
{
	if (IsValid(GoldText))
	{
		GoldText->SetText(FText::AsNumber(NewGold));
	}
}

float UPBGlobalToolbarWidget::GetConfiguredToolbarHeight() const
{
	const UPBUISettings* UISettings = GetDefault<UPBUISettings>();
	return UISettings ? UISettings->GlobalToolbarHeight : 64.0f;
}

bool UPBGlobalToolbarWidget::HasDeckButton() const
{
	return IsValid(DeckButton.Get());
}

TSharedRef<SWidget> UPBGlobalToolbarWidget::RebuildWidget()
{
	// Blueprint subclasses own presentation. The native Slate bar remains a fallback
	// so a missing or unloaded configured Blueprint never removes the global toolbar.
	if (IsValid(WidgetTree) && IsValid(WidgetTree->RootWidget))
	{
		EnsureAnchoredToolbarContent();
		return Super::RebuildWidget();
	}

	const UPBUISettings* UISettings = GetDefault<UPBUISettings>();
	const FLinearColor ToolbarColor = UISettings
		? UISettings->GlobalToolbarColor
		: FLinearColor(0.025f, 0.060f, 0.078f, 0.90f);

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(GetConfiguredToolbarHeight())
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(ToolbarColor)
					.Padding(0.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						[
							BuildLeftSection()
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							BuildCenterSection()
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.HAlign(HAlign_Right)
						.VAlign(VAlign_Center)
						[
							BuildRightSection()
						]
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				BuildLowerSection()
			]
		];
}

void UPBGlobalToolbarWidget::EnsureAnchoredToolbarContent()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!RootCanvas)
	{
		RootCanvas = Cast<UCanvasPanel>(WidgetTree->FindWidget(TEXT("RootCanvas")));
	}
	if (!RootCanvas)
	{
		return;
	}

	EnsureToolbarButtons();
	EnsureRelicInventory();
}
void UPBGlobalToolbarWidget::EnsureToolbarButtons()
{
	if (!RightSection)
	{
		RightSection = Cast<UHorizontalBox>(WidgetTree->FindWidget(TEXT("RightSection")));
	}

	if (USizeBox* OptionButtonSizeBox = Cast<USizeBox>(
		WidgetTree->FindWidget(TEXT("OptionButtonSizeBox"))))
	{
		OptionButtonSizeBox->SetWidthOverride(DeckButtonSize);
		OptionButtonSizeBox->SetHeightOverride(DeckButtonSize);
	}

	// The option widget is shared with the main menu, so override only the toolbar
	// instance instead of replacing its shared source texture.
	if (UUserWidget* OptionButtonWidget = Cast<UUserWidget>(
		WidgetTree->FindWidget(TEXT("OptionButtonWidget"))))
	{
		if (UButton* OptionButton = Cast<UButton>(
			OptionButtonWidget->GetWidgetFromName(TEXT("Button_33"))))
		{
			ApplyToolbarButtonStyle(OptionButton);
		}
		if (UImage* OptionIconImage = Cast<UImage>(
			OptionButtonWidget->GetWidgetFromName(TEXT("Image_22"))))
		{
			if (OptionButtonIcon)
			{
				OptionIconImage->SetBrushFromTexture(OptionButtonIcon, true);
			}
			const float IconSize = FMath::Max(DeckButtonSize - 6.0f, 1.0f);
			OptionIconImage->SetDesiredSizeOverride(FVector2D(IconSize));
			OptionIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	bool bCreatedRightSection = false;
	if (!RightSection)
	{
		RightSection = WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(),
			TEXT("ToolbarRightControls"));
		bCreatedRightSection = IsValid(RightSection);
	}
	if (!RightSection)
	{
		return;
	}

	// Reuse the temporary designer slot in place. This intentionally preserves the
	// Blueprint-authored anchors, child order, spacing, and horizontal-box slot data.
	USizeBox* DeckButtonSizeBox = Cast<USizeBox>(
		WidgetTree->FindWidget(TEXT("DeckButtonSizeBox_Temp")));

	if (!DeckButton)
	{
		DeckButton = Cast<UButton>(WidgetTree->FindWidget(TEXT("DeckButton")));
	}
	if (!DeckButton && DeckButtonSizeBox)
	{
		DeckButton = Cast<UButton>(DeckButtonSizeBox->GetContent());
	}
	if (!DeckButton)
	{
		DeckButton = Cast<UButton>(WidgetTree->FindWidget(TEXT("Button_172")));
	}
	if (!DeckButtonSizeBox && DeckButton)
	{
		DeckButtonSizeBox = Cast<USizeBox>(DeckButton->GetParent());
	}
	if (!DeckButton)
	{
		DeckButton = WidgetTree->ConstructWidget<UButton>(
			UButton::StaticClass(),
			TEXT("DeckButton"));
	}
	if (!DeckButtonSizeBox)
	{
		DeckButtonSizeBox = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			TEXT("ToolbarDeckButtonSizeBox"));
	}

	if (!DeckButtonSizeBox || !DeckButton)
	{
		DeckButton = nullptr;
		return;
	}

	DeckButtonSizeBox->SetWidthOverride(DeckButtonSize);
	DeckButtonSizeBox->SetHeightOverride(DeckButtonSize);
	DeckButton->SetToolTipText(NSLOCTEXT("PBGlobalToolbar", "DeckButtonTooltip", "덱 열기/닫기"));
	ApplyToolbarButtonStyle(DeckButton);

	UImage* DeckIconImage = Cast<UImage>(
		WidgetTree->FindWidget(TEXT("DeckButtonIcon")));
	if (!DeckIconImage && !DeckButton->GetContent())
	{
		DeckIconImage = WidgetTree->ConstructWidget<UImage>(
			UImage::StaticClass(),
			TEXT("DeckButtonIcon"));
	}
	if (DeckIconImage)
	{
		if (DeckButtonIcon)
		{
			DeckIconImage->SetBrushFromTexture(DeckButtonIcon, true);
		}
		const float IconSize = FMath::Max(DeckButtonSize - 6.0f, 1.0f);
		DeckIconImage->SetDesiredSizeOverride(FVector2D(IconSize));
		DeckIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (!DeckIconImage->GetParent())
		{
			DeckButton->SetContent(DeckIconImage);
		}
	}

	if (DeckButtonSizeBox->GetContent() != DeckButton)
	{
		DeckButtonSizeBox->SetContent(DeckButton);
	}
	if (DeckButtonSizeBox->GetParent() != RightSection)
	{
		UHorizontalBoxSlot* DeckSlot = Cast<UHorizontalBoxSlot>(
			RightSection->AddChild(DeckButtonSizeBox));
		if (DeckSlot)
		{
			DeckSlot->SetPadding(FMargin(0.0f, 0.0f, 4.0f, 0.0f));
			DeckSlot->SetHorizontalAlignment(HAlign_Center);
			DeckSlot->SetVerticalAlignment(VAlign_Center);
		}
	}

	if (bCreatedRightSection)
	{
		if (UCanvasPanelSlot* ControlsSlot = Cast<UCanvasPanelSlot>(
			RootCanvas->AddChild(RightSection)))
		{
			ControlsSlot->SetAnchors(FAnchors(1.0f, 0.0f));
			ControlsSlot->SetAlignment(FVector2D(1.0f, 0.0f));
			ControlsSlot->SetAutoSize(true);
			ControlsSlot->SetPosition(FVector2D(-4.0f, 4.0f));
		}
	}
}

void UPBGlobalToolbarWidget::EnsureRelicInventory()
{
	if (RelicInventoryWidget)
	{
		return;
	}

	const TSubclassOf<UPBRelicInventoryWidget> InventoryClass =
		RelicInventoryWidgetClass.LoadSynchronous();
	if (!InventoryClass)
	{
		return;
	}

	RelicInventoryWidget = WidgetTree->ConstructWidget<UPBRelicInventoryWidget>(
		InventoryClass,
		TEXT("ToolbarRelicInventory"));
	if (!RelicInventoryWidget)
	{
		return;
	}

	RelicInventoryWidget->SetManagedByGlobalToolbar();
	RelicInventoryWidget->SetIconSize(RelicIconSize);
	if (UCanvasPanelSlot* RelicSlot = Cast<UCanvasPanelSlot>(
		RootCanvas->AddChild(RelicInventoryWidget)))
	{
		RelicSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		RelicSlot->SetAlignment(FVector2D::ZeroVector);
		RelicSlot->SetAutoSize(true);
		RelicSlot->SetPosition(FVector2D(4.0f, GetConfiguredToolbarHeight() + 2.0f));
	}
}

TSharedRef<SWidget> UPBGlobalToolbarWidget::BuildLeftSection()
{
	return SNullWidget::NullWidget;
}

TSharedRef<SWidget> UPBGlobalToolbarWidget::BuildCenterSection()
{
	return SNullWidget::NullWidget;
}

TSharedRef<SWidget> UPBGlobalToolbarWidget::BuildRightSection()
{
	return SNullWidget::NullWidget;
}

TSharedRef<SWidget> UPBGlobalToolbarWidget::BuildLowerSection()
{
	return SNullWidget::NullWidget;
}
