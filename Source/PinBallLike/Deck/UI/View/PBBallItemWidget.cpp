// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallItemWidget.h"

#include "AssetRegistry/AssetData.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/AssetManager.h"
#include "Engine/Texture2D.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "InputCoreTypes.h"
#include "PBBallDragDropOperation.h"
#include "PinBallLike/Actor/Ball/UI/PBBallDetailTooltipWidget.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Deck/UI/ViewModel/PBBallItemViewModel.h"
#include "PinBallLike/Struct/Ball/PBBallClassType.h"
#include "PinBallLike/Struct/Ball/PBBallDetailTooltipViewData.h"
#include "PinBallLike/Struct/Ball/PBBallRaceType.h"
#include "PinBallLike/Struct/Ball/PBPowerFlipType.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "PinBallLike/Struct/Deck/PBDeckDragMessage.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Table/Ball/PBBallAssetIds.h"
#include "PinBallLike/Table/Ball/Struct/PBBallSkillTableRow.h"
#include "PinBallLike/Table/Ball/Struct/PBBallStarLevelRow.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"
#include "View/MVVMView.h"

void UPBBallItemWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureItemViewModel();
}

void UPBBallItemWidget::InitializeBallItem(const FPBBallItemViewData& InViewData)
{
	ViewData = InViewData;
	EnsureItemViewModel();
	if (ItemViewModel)
	{
		ItemViewModel->SetBallItemViewData(ViewData);
	}

	RebuildBallDetailTooltip();

	UE_LOG(LogTemp, Warning, TEXT("BallItemWidget InitializeBallItem. Widget=%s BallInstanceId=%d BallId=%s StarLevel=%d SlotType=%d SlotIndex=%d Icon=%s"),
		*GetNameSafe(this),
		ViewData.BallInstanceId,
		*ViewData.BallId.ToString(),
		ViewData.StarLevel,
		static_cast<int32>(ViewData.SourceSlotType),
		ViewData.SourceSlotIndex,
		*GetNameSafe(ViewData.Icon));
}

void UPBBallItemWidget::SetSourceSlot(EPBBallDeckSlotType InSourceSlotType, int32 InSourceSlotIndex)
{
	ViewData.SourceSlotType = InSourceSlotType;
	ViewData.SourceSlotIndex = InSourceSlotIndex;
	if (ItemViewModel)
	{
		ItemViewModel->SetBallItemViewData(ViewData);
	}

	RebuildBallDetailTooltip();
}

void UPBBallItemWidget::EnsureItemViewModel()
{
	if (!ItemViewModel)
	{
		ItemViewModel = NewObject<UPBBallItemViewModel>(this);
	}

	if (ItemViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBBallItemWidget::ApplyViewModelToWidget()
{
	if (!ItemViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallItemWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(ItemViewModel);
	const bool bResult = View->SetViewModelByClass(ViewModelInterface);
	if (!bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallItemWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(ItemViewModel));
	}
	return bResult;
}

FReply UPBBallItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (ViewData.BallInstanceId == INDEX_NONE)
	{
		return Reply;
	}

	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

void UPBBallItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (ViewData.BallInstanceId == INDEX_NONE)
	{
		return;
	}

	UPBBallDragDropOperation* DragDropOperation = NewObject<UPBBallDragDropOperation>(this);
	if (!DragDropOperation)
	{
		return;
	}

	DragDropOperation->InitializeBallDrag(ViewData.BallInstanceId, ViewData.SourceSlotType, ViewData.SourceSlotIndex);
	DragDropOperation->Pivot = EDragPivot::MouseDown;
	DragDropOperation->Payload = this;

	if (UPBBallItemWidget* DragVisualWidget = CreateWidget<UPBBallItemWidget>(this, GetClass()))
	{
		DragVisualWidget->InitializeBallItem(ViewData);
		DragVisualWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		DragDropOperation->DefaultDragVisual = DragVisualWidget;
	}

	OutOperation = DragDropOperation;

	FPBDeckDragStartedMessage Message;
	Message.ItemId = ViewData.BallInstanceId;
	Message.SourceObject = this;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_UI_Deck_Drag_Started,
		Message);
}

void UPBBallItemWidget::RebuildBallDetailTooltip()
{
	if (!ViewData.IsValid())
	{
		SetToolTip(nullptr);
		return;
	}

	if (!BallDetailTooltipWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallItemWidget tooltip skipped. Tooltip class is null. Widget=%s BallId=%s"),
			*GetNameSafe(this),
			*ViewData.BallId.ToString());
		SetToolTip(nullptr);
		return;
	}

	FPBBallDetailTooltipViewData TooltipViewData;
	if (!BuildBallDetailTooltipViewData(TooltipViewData))
	{
		SetToolTip(nullptr);
		return;
	}

	UPBBallDetailTooltipWidget* TooltipWidget = CreateWidget<UPBBallDetailTooltipWidget>(
		GetOwningPlayer(),
		BallDetailTooltipWidgetClass);
	if (!TooltipWidget)
	{
		SetToolTip(nullptr);
		return;
	}

	TooltipWidget->SetTooltipViewData(TooltipViewData);
	SetToolTip(TooltipWidget);
}

bool UPBBallItemWidget::BuildBallDetailTooltipViewData(FPBBallDetailTooltipViewData& OutTooltipViewData) const
{
	OutTooltipViewData = FPBBallDetailTooltipViewData();
	if (!ViewData.IsValid())
	{
		return false;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableDataSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBTableDataSubsystem>() : nullptr;
	if (!TableDataSubsystem)
	{
		return false;
	}

	FPBBallTableRow BallRow;
	if (!TableDataSubsystem->FindBallRow(ViewData.BallId, BallRow))
	{
		return false;
	}

	FName StarLevelRowName = NAME_None;
	FPBBallStarLevelRow StarLevelRow;
	if (!TableDataSubsystem->FindBallStarLevelRow(
		ViewData.BallId,
		ViewData.StarLevel,
		StarLevelRowName,
		StarLevelRow))
	{
		return false;
	}

	FPBBallSkillTableRow SkillRow;
	(void)TableDataSubsystem->FindDefaultSkillRowForBall(ViewData.BallId, SkillRow);

	const UEnum* PowerFlipEnum = StaticEnum<EPBPowerFlipType>();
	const UEnum* RaceEnum = StaticEnum<EPBBallRaceType>();
	const UEnum* ClassEnum = StaticEnum<EPBBallClassType>();

	const FPrimaryAssetId BallAssetId(PBBallAssetIds::Type::BallData, ViewData.BallId);
	FAssetData BallAssetData;
	const UPBBallDataAsset* BallDataAsset = nullptr;
	if (UAssetManager::Get().GetPrimaryAssetData(BallAssetId, BallAssetData))
	{
		BallDataAsset = Cast<UPBBallDataAsset>(BallAssetData.GetAsset());
	}

	OutTooltipViewData.bHasBall = true;
	OutTooltipViewData.BallId = ViewData.BallId;
	OutTooltipViewData.BallIconTexture = ViewData.Icon;
	if (!OutTooltipViewData.BallIconTexture && BallDataAsset)
	{
		OutTooltipViewData.BallIconTexture = BallDataAsset->BallIcon.LoadSynchronous();
	}
	OutTooltipViewData.BallNameText = BallRow.DisplayName.IsEmpty()
		? FText::FromName(ViewData.BallId)
		: BallRow.DisplayName;
	OutTooltipViewData.BallDescriptionText = BallRow.DescriptionKey;

	OutTooltipViewData.HpRow = MakeInfoRow(
		NSLOCTEXT("BallDetailTooltip", "HPLabel", "HP"),
		FindMapValue(StarLevelRow.BaseResources, PBResourceNames::Health));
	OutTooltipViewData.MpRow = MakeInfoRow(
		NSLOCTEXT("BallDetailTooltip", "MPLabel", "MP"),
		FindMapValue(StarLevelRow.BaseResources, PBResourceNames::Mana));
	OutTooltipViewData.AttackRow = MakeInfoRow(
		NSLOCTEXT("BallDetailTooltip", "AttackLabel", "공격력"),
		FindMapValue(StarLevelRow.BaseStats, PBStatNames::Attack));
	OutTooltipViewData.ManaRegenRow = MakeInfoRow(
		NSLOCTEXT("BallDetailTooltip", "ManaRegenLabel", "MP회복"),
		FindMapValue(StarLevelRow.BaseStats, PBStatNames::ManaRegen));

	OutTooltipViewData.PowerFlipData = MakeIconText(
		BallDataAsset ? BallDataAsset->PowerFlipIcon.LoadSynchronous() : nullptr,
		GetEnumDisplayText(PowerFlipEnum, static_cast<int64>(BallRow.PowerFlipType)));

	if (BallRow.ClassType != EPBBallClassType::None)
	{
		OutTooltipViewData.ClassData = MakeIconText(
			BallDataAsset ? BallDataAsset->ClassIcon.LoadSynchronous() : nullptr,
			GetEnumDisplayText(ClassEnum, static_cast<int64>(BallRow.ClassType)));
	}

	for (int32 RaceIndex = 0; RaceIndex < BallRow.RaceTypes.Num(); ++RaceIndex)
	{
		const EPBBallRaceType RaceType = BallRow.RaceTypes[RaceIndex];
		const FName RaceSynergyId = GetEnumValueName(RaceEnum, static_cast<int64>(RaceType));
		if (RaceSynergyId.IsNone())
		{
			continue;
		}

		OutTooltipViewData.RaceDataList.Add(MakeIconText(
			(BallDataAsset && BallDataAsset->RaceIcons.IsValidIndex(RaceIndex))
				? BallDataAsset->RaceIcons[RaceIndex].LoadSynchronous()
				: nullptr,
			GetEnumDisplayText(RaceEnum, static_cast<int64>(RaceType))));
	}

	OutTooltipViewData.SkillNameText = SkillRow.DisplayName;
	OutTooltipViewData.SkillDescriptionText = SkillRow.Description;
	if (BallDataAsset)
	{
		OutTooltipViewData.SkillIconTexture = BallDataAsset->SkillIcon.LoadSynchronous();
	}

	return true;
}

FPBBallDetailInfoRowViewData UPBBallItemWidget::MakeInfoRow(const FText& LabelText, const int32 Value) const
{
	FPBBallDetailInfoRowViewData RowViewData;
	RowViewData.LabelText = LabelText;
	RowViewData.ValueText = FText::AsNumber(Value);
	return RowViewData;
}

FPBBallDetailIconTextViewData UPBBallItemWidget::MakeIconText(UTexture2D* IconTexture, const FText& Text) const
{
	FPBBallDetailIconTextViewData ViewData;
	ViewData.IconTexture = IconTexture;
	ViewData.Text = Text;
	return ViewData;
}

int32 UPBBallItemWidget::FindMapValue(const TMap<FName, int32>& Values, const FName Key) const
{
	const int32* Value = Values.Find(Key);
	return Value ? *Value : 0;
}

FText UPBBallItemWidget::GetEnumDisplayText(const UEnum* Enum, const int64 Value) const
{
	return Enum ? Enum->GetDisplayNameTextByValue(Value) : FText::GetEmpty();
}

FName UPBBallItemWidget::GetEnumValueName(const UEnum* Enum, const int64 Value) const
{
	return Enum ? FName(*Enum->GetNameStringByValue(Value)) : NAME_None;
}
