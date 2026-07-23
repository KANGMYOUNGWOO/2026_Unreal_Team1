// Fill out your copyright notice in the Description page of Project Settings.
#include "PBRelicIconWidget.h"

#include "PinBallLike/Relic/UI/PBRelicTooltipWidget.h"
#include "PinBallLike/Relic/UI/PBRelicViewModel.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Relic/Struct/PBRelicTableRow.h"
#include "View/MVVMView.h"

void UPBRelicIconWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureRelicViewModel();
}

void UPBRelicIconWidget::SetRelicId(FName InRelicId)
{
	SetRelicViewData(BuildRelicViewData(InRelicId));
}

void UPBRelicIconWidget::SetRelicViewData(const FPBRelicViewData& InViewData)
{
	ViewData = InViewData;
	EnsureRelicViewModel();
	if (RelicViewModel)
	{
		RelicViewModel->SetRelicViewData(ViewData);
	}
	RebuildRelicTooltip();
}

FPBRelicViewData UPBRelicIconWidget::BuildRelicViewData(FName InRelicId) const
{
	FPBRelicViewData NewViewData;
	NewViewData.RelicId = InRelicId;

	if (InRelicId.IsNone())
	{
		return NewViewData;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return NewViewData;
	}

	UPBTableDataSubsystem* TableSubsystem = GameInstance->GetSubsystem<UPBTableDataSubsystem>();
	if (!TableSubsystem)
	{
		return NewViewData;
	}

	FPBRelicTableRow RelicRow;
	if (!TableSubsystem->FindRelicRow(InRelicId, RelicRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("[RelicIcon] Relic row not found. RelicId=%s"), *InRelicId.ToString());
		return NewViewData;
	}

	if (!RelicRow.RelicDataAsset.IsNull())
	{
		if (UPBRelicDataAsset* DataAsset = RelicRow.RelicDataAsset.LoadSynchronous())
		{
			NewViewData.IconTexture = DataAsset->RelicIcon.LoadSynchronous();
		}
	}
	
	NewViewData.DisplayName = RelicRow.DisplayName.IsEmpty() ? FText::FromName(InRelicId) : RelicRow.DisplayName;
	NewViewData.Description = RelicRow.Description;
	NewViewData.Rarity = RelicRow.Rarity;
	return NewViewData;
}

void UPBRelicIconWidget::RebuildRelicTooltip()
{
	if (!ViewData.IsValid())
	{
		SetToolTip(nullptr);
		return;
	}

	if (!RelicTooltipWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RelicIcon] Tooltip class is null. RelicId=%s"), *ViewData.RelicId.ToString());
		SetToolTip(nullptr);
		return;
	}

	UPBRelicTooltipWidget* TooltipWidget = CreateWidget<UPBRelicTooltipWidget>(GetOwningPlayer(), RelicTooltipWidgetClass);
	if (!TooltipWidget)
	{
		return;
	}

	TooltipWidget->SetRelicViewData(ViewData);
	SetToolTip(TooltipWidget);
	UE_LOG(LogTemp, Warning, TEXT("[RelicIcon] Tooltip assigned. RelicId=%s Tooltip=%s"), *ViewData.RelicId.ToString(), *GetNameSafe(GetToolTip()));
}

void UPBRelicIconWidget::EnsureRelicViewModel()
{
	if (!RelicViewModel)
	{
		RelicViewModel = NewObject<UPBRelicViewModel>(this);
	}

	if (RelicViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBRelicIconWidget::ApplyViewModelToWidget()
{
	if (!RelicViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("RelicIconWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"), *GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(RelicViewModel);
	const bool bResult = View->SetViewModelByClass(ViewModelInterface);
	if (!bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("RelicIconWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"), *GetNameSafe(this), *GetNameSafe(RelicViewModel));
	}
	return bResult;
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
		*ViewData.RelicId.ToString());
}

void UPBRelicIconWidget::NativeOnMouseLeave(
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[RelicIcon] Mouse Leave. RelicId=%s"),
		*ViewData.RelicId.ToString());
}
