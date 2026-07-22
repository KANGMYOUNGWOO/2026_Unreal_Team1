#include "PBGlobalToolbarWidget.h"

#include "PinBallLike/DeveloperSettings/PBUISettings.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"

float UPBGlobalToolbarWidget::GetConfiguredToolbarHeight() const
{
	const UPBUISettings* UISettings = GetDefault<UPBUISettings>();
	return UISettings ? UISettings->GlobalToolbarHeight : 64.0f;
}

TSharedRef<SWidget> UPBGlobalToolbarWidget::RebuildWidget()
{
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
