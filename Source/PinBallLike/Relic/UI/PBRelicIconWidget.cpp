// Fill out your copyright notice in the Description page of Project Settings.
#include "PBRelicIconWidget.h"

#include "PinBallLike/Table/Relic/Struct/PBRelicTableRow.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Relic/UI/PBRelicTooltipWidget.h"


void UPBRelicIconWidget::SetRelicId(FName InRelicId)
{
	RelicId = InRelicId;

	UGameInstance* GameInstance =
		GetGameInstance();

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
		return;
	}

	if (!RelicTooltipWidgetClass)
	{
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

	SetToolTip(
		TooltipWidget);
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