// Fill out your copyright notice in the Description page of Project Settings.
#include "PBRelicIconWidget.h"

#include "PinBallLike/Table/Relic/Struct/PBRelicTableRow.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Relic/UI/PBRelicTooltipWidget.h"


void UPBRelicIconWidget::SetRelicId(FName InRelicId)
{
	RelicId = InRelicId;

	if (RelicId.IsNone())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UPBTableDataSubsystem* TableSubsystem =
		GameInstance->GetSubsystem<UPBTableDataSubsystem>();

	if (!TableSubsystem)
	{
		return;
	}

	FPBRelicTableRow RelicRow;

	if (!TableSubsystem->FindRelicRow(
		RelicId,
		RelicRow))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[RelicIcon] Relic row not found. RelicId=%s"),
			*RelicId.ToString());

		return;
	}

	if (!RelicTooltipWidgetClass)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[RelicIcon] Tooltip class is null. RelicId=%s"),
			*RelicId.ToString());

		return;
	}

	UPBRelicTooltipWidget* TooltipWidget =
		CreateWidget<UPBRelicTooltipWidget>(
			GetOwningPlayer(),
			RelicTooltipWidgetClass);

	if (!TooltipWidget)
	{
		return;
	}

	TooltipWidget->SetRelicData(
		RelicRow.DisplayName,
		RelicRow.Description);

	SetToolTip(TooltipWidget);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[RelicIcon] Tooltip assigned. RelicId=%s Tooltip=%s"),
		*RelicId.ToString(),
		*GetNameSafe(GetToolTip()));
}

void UPBRelicIconWidget::NativeOnMouseEnter(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[RelicIcon] Mouse Enter. RelicId=%s"),
		*RelicId.ToString());
}

void UPBRelicIconWidget::NativeOnMouseLeave(
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[RelicIcon] Mouse Leave. RelicId=%s"),
		*RelicId.ToString());
}