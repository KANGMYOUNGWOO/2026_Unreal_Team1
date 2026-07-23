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
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"

UPBGlobalToolbarWidget::UPBGlobalToolbarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DeckButtonIcon(FSoftObjectPath(TEXT("/Game/Resources/Deck/Deck.Deck")))
	, RelicInventoryWidgetClass(FSoftObjectPath(
		TEXT("/Game/Blueprints/UI/Relic/WBP_PBRelicInventoryWidget.WBP_PBRelicInventoryWidget_C")))
{
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

	EnsureRightControls();
	EnsureRelicInventory();
}

void UPBGlobalToolbarWidget::EnsureRightControls()
{
	if (!RightSection)
	{
		RightSection = Cast<UHorizontalBox>(WidgetTree->FindWidget(TEXT("RightSection")));
	}
	if (!OptionButtonSizeBox)
	{
		OptionButtonSizeBox = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("OptionButtonSizeBox")));
	}
	if (!OptionButtonWidget)
	{
		OptionButtonWidget = WidgetTree->FindWidget(TEXT("OptionButtonWidget"));
	}
	if (!DeckButton)
	{
		DeckButton = Cast<UButton>(WidgetTree->FindWidget(TEXT("DeckButton")));
	}

	if (RightControls)
	{
		return;
	}

	RightControls = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(),
		TEXT("ToolbarRightControls"));
	USizeBox* DeckButtonSizeBox = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(),
		TEXT("DeckButtonSizeBox"));
	if (!DeckButton)
	{
		DeckButton = WidgetTree->ConstructWidget<UButton>(
			UButton::StaticClass(),
			TEXT("DeckButton"));
	}
	UImage* DeckIconImage = WidgetTree->ConstructWidget<UImage>(
		UImage::StaticClass(),
		TEXT("DeckButtonIcon"));

	if (!RightControls || !DeckButtonSizeBox || !DeckButton || !DeckIconImage)
	{
		DeckButton = nullptr;
		RightControls = nullptr;
		return;
	}

	DeckButtonSizeBox->SetWidthOverride(DeckButtonSize);
	DeckButtonSizeBox->SetHeightOverride(DeckButtonSize);
	DeckButton->SetToolTipText(NSLOCTEXT("PBGlobalToolbar", "DeckButtonTooltip", "덱 열기/닫기"));

	if (UTexture2D* IconTexture = DeckButtonIcon.LoadSynchronous())
	{
		DeckIconImage->SetBrushFromTexture(IconTexture, true);
	}
	DeckIconImage->SetDesiredSizeOverride(FVector2D(DeckButtonSize - 8.0f));
	DeckIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	DeckButton->AddChild(DeckIconImage);
	DeckButtonSizeBox->AddChild(DeckButton);

	UHorizontalBoxSlot* DeckButtonSlot = Cast<UHorizontalBoxSlot>(
		RightControls->AddChild(DeckButtonSizeBox));
	if (DeckButtonSlot)
	{
		DeckButtonSlot->SetPadding(FMargin(0.0f, 0.0f, 4.0f, 0.0f));
		DeckButtonSlot->SetHorizontalAlignment(HAlign_Center);
		DeckButtonSlot->SetVerticalAlignment(VAlign_Center);
	}

	if (OptionButtonSizeBox)
	{
		OptionButtonSizeBox->RemoveFromParent();
		OptionButtonSizeBox->SetWidthOverride(DeckButtonSize);
		OptionButtonSizeBox->SetHeightOverride(DeckButtonSize);
		if (UHorizontalBoxSlot* OptionSlot = Cast<UHorizontalBoxSlot>(
			RightControls->AddChild(OptionButtonSizeBox)))
		{
			OptionSlot->SetHorizontalAlignment(HAlign_Center);
			OptionSlot->SetVerticalAlignment(VAlign_Center);
		}
	}

	if (UCanvasPanelSlot* ControlsSlot = Cast<UCanvasPanelSlot>(
		RootCanvas->AddChild(RightControls)))
	{
		ControlsSlot->SetAnchors(FAnchors(1.0f, 0.0f));
		ControlsSlot->SetAlignment(FVector2D(1.0f, 0.0f));
		ControlsSlot->SetAutoSize(true);
		ControlsSlot->SetPosition(FVector2D(-4.0f, 4.0f));
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
