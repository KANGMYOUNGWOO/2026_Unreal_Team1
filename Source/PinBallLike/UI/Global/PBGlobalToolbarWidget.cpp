#include "PBGlobalToolbarWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "PinBallLike/DeveloperSettings/PBUISettings.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"

void UPBGlobalToolbarWidget::NativeConstruct()
{
	Super::NativeConstruct();

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
	if (IsValid(PlayerDataSubsystem))
	{
		PlayerDataSubsystem->OnGoldChanged.RemoveDynamic(
			this,
			&UPBGlobalToolbarWidget::HandleGoldChanged);
	}
	PlayerDataSubsystem = nullptr;

	Super::NativeDestruct();
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

TSharedRef<SWidget> UPBGlobalToolbarWidget::RebuildWidget()
{
	// Blueprint subclasses own presentation. The native Slate bar remains a fallback
	// so a missing or unloaded configured Blueprint never removes the global toolbar.
	if (IsValid(WidgetTree) && IsValid(WidgetTree->RootWidget))
	{
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
