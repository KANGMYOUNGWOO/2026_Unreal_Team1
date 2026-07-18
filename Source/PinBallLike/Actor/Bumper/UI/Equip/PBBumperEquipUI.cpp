#include "PBBumperEquipUI.h"

#include "PBBumperDragDropOperation.h"
#include "PBBumperEquipController.h"
#include "PBBumperEquipUIBuilder.h"
#include "PinBallLike/Actor/Bumper/UI/Equip/PBBumperInfoPanelViewModel.h"
#include "PinBallLike/Actor/Bumper/UI/Equip/PBBumperListItemObject.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Utils/PBTextFormatUtils.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Blueprint/WidgetTree.h"

namespace
{
	constexpr int32 TopCatalogIndex = 0;
	constexpr int32 SideCatalogIndex = 1;
	constexpr int32 ReboundCatalogIndex = 2;
	constexpr int32 SpecialCatalogIndex = 3;
	constexpr int32 BumperEquipSlotCount = 7;
}

void UPBBumperEquipUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BindBoardSlotButtons();
	BindRedesignedControls();
	EnsureEquipController();
	BindControllerEvents();
	if (IsValid(EquipController))
	{
		EquipController->Initialize(GetGameInstance());
	}
}

void UPBBumperEquipUI::NativeConstruct()
{
	Super::NativeConstruct();
	bWidgetConstructed = true;
	bCatalogReadyBroadcastForConstruct = false;
	EnsureEquipController();
	BindControllerEvents();
	if (IsValid(EquipController))
	{
		EquipController->Initialize(GetGameInstance());
	}
	RefreshRedesignedPresentation();

	// 재사용된 위젯은 컨트롤러가 이미 만든 목록을 즉시 Blueprint 목록에 다시 전달합니다.
	if (IsValid(EquipController) && EquipController->IsCatalogReady())
	{
		HandleControllerCatalogReady();
	}
}

void UPBBumperEquipUI::NativeDestruct()
{
	bWidgetConstructed = false;
	bCatalogReadyBroadcastForConstruct = false;
	HoveredDropSlot.Reset();
	UnbindControllerEvents();
	if (IsValid(EquipController))
	{
		EquipController->Shutdown();
	}

	Super::NativeDestruct();
}

void UPBBumperEquipUI::NativeOnDragLeave(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	SetHoveredDropSlot(TOptional<EPBBumperEquipSlot>());
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
}

bool UPBBumperEquipUI::NativeOnDragOver(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const UPBBumperDragDropOperation* DragOperation = Cast<UPBBumperDragDropOperation>(InOperation);
	if (!IsValid(DragOperation) || !DragOperation->IsValidBumperDrag())
	{
		SetHoveredDropSlot(TOptional<EPBBumperEquipSlot>());
		return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
	}

	EPBBumperEquipSlot TargetSlot;
	const bool bCanDrop = FindBoardSlotAtScreenPosition(
		InDragDropEvent.GetScreenSpacePosition(),
		TargetSlot)
		&& CanEquipBumperRowAtSlot(DragOperation->BumperRowName, TargetSlot);

	SetHoveredDropSlot(bCanDrop
		? TOptional<EPBBumperEquipSlot>(TargetSlot)
		: TOptional<EPBBumperEquipSlot>());
	return bCanDrop;
}

bool UPBBumperEquipUI::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const UPBBumperDragDropOperation* DragOperation = Cast<UPBBumperDragDropOperation>(InOperation);
	if (!IsValid(DragOperation) || !DragOperation->IsValidBumperDrag())
	{
		SetHoveredDropSlot(TOptional<EPBBumperEquipSlot>());
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	EPBBumperEquipSlot TargetSlot;
	const bool bCanDrop = FindBoardSlotAtScreenPosition(
		InDragDropEvent.GetScreenSpacePosition(),
		TargetSlot)
		&& CanEquipBumperRowAtSlot(DragOperation->BumperRowName, TargetSlot);
	SetHoveredDropSlot(TOptional<EPBBumperEquipSlot>());

	return bCanDrop && EquipBumperRowAtSlot(DragOperation->BumperRowName, TargetSlot);
}

void UPBBumperEquipUI::GetBumperListItemObjects(
	TArray<UPBBumperListItemObject*>& OutTopItems,
	TArray<UPBBumperListItemObject*>& OutSideItems,
	TArray<UPBBumperListItemObject*>& OutReboundItems,
	TArray<UPBBumperListItemObject*>& OutSpecialItems) const
{
	if (IsValid(EquipController))
	{
		EquipController->GetBumperListItemObjects(
			OutTopItems,
			OutSideItems,
			OutReboundItems,
			OutSpecialItems);
		return;
	}

	OutTopItems.Reset();
	OutSideItems.Reset();
	OutReboundItems.Reset();
	OutSpecialItems.Reset();
}

bool UPBBumperEquipUI::SelectBumperSlot(const EPBBumperSlotType SlotType)
{
	return IsValid(EquipController) && EquipController->SelectBumperSlot(SlotType);
}

bool UPBBumperEquipUI::SelectBumperEquipSlot(const EPBBumperEquipSlot EquipSlot)
{
	return IsValid(EquipController) && EquipController->SelectBumperEquipSlot(EquipSlot);
}

void UPBBumperEquipUI::SelectBumperRow(const FName RowName)
{
	if (IsValid(EquipController))
	{
		EquipController->SelectBumperRow(RowName);
	}
}

bool UPBBumperEquipUI::GetEquippedBumperForSlot(
	const EPBBumperSlotType SlotType,
	FName& OutBumperRowId) const
{
	if (!IsValid(EquipController))
	{
		OutBumperRowId = NAME_None;
		return false;
	}
	return EquipController->GetEquippedBumperForSlot(SlotType, OutBumperRowId);
}

bool UPBBumperEquipUI::GetEquippedBumperForEquipSlot(
	const EPBBumperEquipSlot EquipSlot,
	FName& OutBumperRowId) const
{
	if (!IsValid(EquipController))
	{
		OutBumperRowId = NAME_None;
		return false;
	}
	return EquipController->GetEquippedBumperForEquipSlot(EquipSlot, OutBumperRowId);
}

EPBBumperSlotType UPBBumperEquipUI::GetSelectedBumperSlotType() const
{
	return IsValid(EquipController)
		? EquipController->GetSelectedBumperSlotType()
		: EPBBumperSlotType::Top;
}

FName UPBBumperEquipUI::GetSelectedBumperRowName() const
{
	return IsValid(EquipController) ? EquipController->GetSelectedBumperRowName() : NAME_None;
}

EPBBumperEquipSlot UPBBumperEquipUI::GetSelectedBumperEquipSlot() const
{
	return IsValid(EquipController)
		? EquipController->GetSelectedBumperEquipSlot()
		: EPBBumperEquipSlot::TopLeft;
}

UPBBumperInfoPanelViewModel* UPBBumperEquipUI::GetInfoPanelViewModel() const
{
	return IsValid(EquipController) ? EquipController->GetInfoPanelViewModel() : nullptr;
}

bool UPBBumperEquipUI::EquipBumperRow(const FName RowName)
{
	return IsValid(EquipController) && EquipController->EquipBumperRow(RowName);
}

bool UPBBumperEquipUI::EquipBumperRowAtSlot(
	const FName RowName,
	const EPBBumperEquipSlot EquipSlot)
{
	return IsValid(EquipController) && EquipController->EquipBumperRowAtSlot(RowName, EquipSlot);
}

bool UPBBumperEquipUI::UnequipBumperRow(const FName RowName)
{
	return IsValid(EquipController) && EquipController->UnequipBumperRow(RowName);
}

bool UPBBumperEquipUI::EquipSelectedBumper()
{
	return IsValid(EquipController) && EquipController->EquipSelectedBumper();
}

bool UPBBumperEquipUI::UnequipSelectedBumper()
{
	return IsValid(EquipController) && EquipController->UnequipSelectedBumper();
}

void UPBBumperEquipUI::EnsureEquipController()
{
	if (!IsValid(EquipController))
	{
		EquipController = NewObject<UPBBumperEquipController>(this);
	}
}

void UPBBumperEquipUI::BindControllerEvents()
{
	if (!IsValid(EquipController))
	{
		return;
	}

	EquipController->OnCatalogReady.RemoveAll(this);
	EquipController->OnSelectionChanged.RemoveAll(this);
	EquipController->OnCatalogReady.AddUObject(this, &ThisClass::HandleControllerCatalogReady);
	EquipController->OnSelectionChanged.AddUObject(this, &ThisClass::HandleControllerSelectionChanged);
}

void UPBBumperEquipUI::UnbindControllerEvents()
{
	if (IsValid(EquipController))
	{
		EquipController->OnCatalogReady.RemoveAll(this);
		EquipController->OnSelectionChanged.RemoveAll(this);
	}
}

void UPBBumperEquipUI::HandleControllerCatalogReady()
{
	if (!bWidgetConstructed || bCatalogReadyBroadcastForConstruct || !IsValid(EquipController))
	{
		return;
	}

	bCatalogReadyBroadcastForConstruct = true;
	OnBumperListItemsReady.Broadcast();
	HandleControllerSelectionChanged();
}

void UPBBumperEquipUI::HandleControllerSelectionChanged()
{
	if (!bWidgetConstructed)
	{
		return;
	}

	BroadcastSelectedSlotChanged();
	RefreshBoardSlotSelection();
	RefreshRedesignedPresentation();
}

void UPBBumperEquipUI::BindBoardSlotButtons()
{
	BuildBoardSlotPresentations();

	if (IsValid(TopLeftMarker))
	{
		TopLeftMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleTopLeftSlotClicked);
	}
	if (IsValid(TopRightMarker))
	{
		TopRightMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleTopRightSlotClicked);
	}
	if (IsValid(SideLeftMarker))
	{
		SideLeftMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleSideLeftSlotClicked);
	}
	if (IsValid(SideRightMarker))
	{
		SideRightMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleSideRightSlotClicked);
	}
	if (IsValid(ReboundLeftMarker))
	{
		ReboundLeftMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleReboundLeftSlotClicked);
	}
	if (IsValid(ReboundRightMarker))
	{
		ReboundRightMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleReboundRightSlotClicked);
	}
	if (IsValid(SpecialCenterMarker))
	{
		SpecialCenterMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleSpecialSlotClicked);
	}

	RefreshBoardSlotSelection();
}

void UPBBumperEquipUI::BuildBoardSlotPresentations()
{
	BoardSlotIconImages.Reset();
	BoardSlotLabels.Reset();

	BuildBoardSlotPresentation(TopLeftMarker, EPBBumperEquipSlot::TopLeft);
	BuildBoardSlotPresentation(TopRightMarker, EPBBumperEquipSlot::TopRight);
	BuildBoardSlotPresentation(SideLeftMarker, EPBBumperEquipSlot::SideLeft);
	BuildBoardSlotPresentation(SideRightMarker, EPBBumperEquipSlot::SideRight);
	BuildBoardSlotPresentation(ReboundLeftMarker, EPBBumperEquipSlot::ReboundLeft);
	BuildBoardSlotPresentation(ReboundRightMarker, EPBBumperEquipSlot::ReboundRight);
	BuildBoardSlotPresentation(SpecialCenterMarker, EPBBumperEquipSlot::Special);
}

void UPBBumperEquipUI::BuildBoardSlotPresentation(
	UButton* Button,
	const EPBBumperEquipSlot EquipSlot)
{
	if (!IsValid(Button) || !IsValid(WidgetTree))
	{
		return;
	}

	// 기존 Blueprint 라벨을 재사용하여 폰트 설정과 변수 참조를 보존합니다.
	UTextBlock* Label = Cast<UTextBlock>(Button->GetContent());
	if (!IsValid(Label) || !Button->RemoveChild(Label))
	{
		return;
	}

	const FString SlotSuffix = FString::FromInt(static_cast<int32>(EquipSlot));
	UVerticalBox* CardContent = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(),
		FName(*FString::Printf(TEXT("BumperSlotCard_%s"), *SlotSuffix)));
	USizeBox* IconSizeBox = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(),
		FName(*FString::Printf(TEXT("BumperSlotIconSize_%s"), *SlotSuffix)));
	UImage* IconImage = WidgetTree->ConstructWidget<UImage>(
		UImage::StaticClass(),
		FName(*FString::Printf(TEXT("BumperSlotIcon_%s"), *SlotSuffix)));
	if (!IsValid(CardContent) || !IsValid(IconSizeBox) || !IsValid(IconImage))
	{
		Button->AddChild(Label);
		return;
	}

	IconSizeBox->SetHeightOverride(98.0f);
	IconSizeBox->AddChild(IconImage);
	IconImage->SetVisibility(ESlateVisibility::Hidden);
	IconImage->SetColorAndOpacity(FLinearColor::White);

	if (UVerticalBoxSlot* IconSlot = CardContent->AddChildToVerticalBox(IconSizeBox))
	{
		IconSlot->SetPadding(FMargin(6.0f, 5.0f, 6.0f, 2.0f));
		IconSlot->SetHorizontalAlignment(HAlign_Fill);
		IconSlot->SetVerticalAlignment(VAlign_Fill);
	}

	Label->SetAutoWrapText(true);
	Label->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* LabelSlot = CardContent->AddChildToVerticalBox(Label))
	{
		LabelSlot->SetPadding(FMargin(5.0f, 2.0f, 5.0f, 5.0f));
		LabelSlot->SetHorizontalAlignment(HAlign_Fill);
		LabelSlot->SetVerticalAlignment(VAlign_Center);
	}

	if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(Button->AddChild(CardContent)))
	{
		ButtonSlot->SetPadding(FMargin(2.0f));
		ButtonSlot->SetHorizontalAlignment(HAlign_Fill);
		ButtonSlot->SetVerticalAlignment(VAlign_Fill);
	}

	BoardSlotIconImages.Add(EquipSlot, IconImage);
	BoardSlotLabels.Add(EquipSlot, Label);
}

void UPBBumperEquipUI::BindRedesignedControls()
{
	if (IsValid(TopCategoryButton))
	{
		TopCategoryButton->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleTopCategoryClicked);
	}
	if (IsValid(SideCategoryButton))
	{
		SideCategoryButton->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleSideCategoryClicked);
	}
	if (IsValid(ReboundCategoryButton))
	{
		ReboundCategoryButton->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleReboundCategoryClicked);
	}
	if (IsValid(SpecialCategoryButton))
	{
		SpecialCategoryButton->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleSpecialCategoryClicked);
	}
	if (IsValid(EquipActionButton))
	{
		EquipActionButton->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleEquipActionClicked);
	}

	ClearDetailPresentation();
}

void UPBBumperEquipUI::RefreshBoardSlotSelection()
{
	SetBoardSlotButtonState(TopLeftMarker, EPBBumperEquipSlot::TopLeft);
	SetBoardSlotButtonState(TopRightMarker, EPBBumperEquipSlot::TopRight);
	SetBoardSlotButtonState(SideLeftMarker, EPBBumperEquipSlot::SideLeft);
	SetBoardSlotButtonState(SideRightMarker, EPBBumperEquipSlot::SideRight);
	SetBoardSlotButtonState(ReboundLeftMarker, EPBBumperEquipSlot::ReboundLeft);
	SetBoardSlotButtonState(ReboundRightMarker, EPBBumperEquipSlot::ReboundRight);
	SetBoardSlotButtonState(SpecialCenterMarker, EPBBumperEquipSlot::Special);
}

void UPBBumperEquipUI::RefreshRedesignedPresentation()
{
	RefreshCategoryPresentation();
	RefreshLoadoutPresentation();

	const FName SelectedRowName = GetSelectedBumperRowName();
	if (SelectedRowName.IsNone())
	{
		ClearDetailPresentation();
	}
	else
	{
		UpdateDetailPresentation(SelectedRowName);
	}
}

void UPBBumperEquipUI::RefreshCategoryPresentation()
{
	const EPBBumperSlotType SelectedSlotType = GetSelectedBumperSlotType();
	int32 CatalogIndex = TopCatalogIndex;
	switch (SelectedSlotType)
	{
	case EPBBumperSlotType::Top:
		CatalogIndex = TopCatalogIndex;
		break;
	case EPBBumperSlotType::Side:
		CatalogIndex = SideCatalogIndex;
		break;
	case EPBBumperSlotType::Rebound:
		CatalogIndex = ReboundCatalogIndex;
		break;
	case EPBBumperSlotType::Special:
		CatalogIndex = SpecialCatalogIndex;
		break;
	default:
		break;
	}

	if (IsValid(CatalogSwitcher) && CatalogSwitcher->GetNumWidgets() > CatalogIndex)
	{
		CatalogSwitcher->SetActiveWidgetIndex(CatalogIndex);
	}

	SetCategoryButtonState(TopCategoryButton, EPBBumperSlotType::Top);
	SetCategoryButtonState(SideCategoryButton, EPBBumperSlotType::Side);
	SetCategoryButtonState(ReboundCategoryButton, EPBBumperSlotType::Rebound);
	SetCategoryButtonState(SpecialCategoryButton, EPBBumperSlotType::Special);

	if (IsValid(CatalogTitleText))
	{
		CatalogTitleText->SetText(FText::Format(
			NSLOCTEXT("PBBumperEquipUI", "CatalogTitleFormat", "{0} 범퍼"),
			PBBumperEquipUIBuilder::GetBumperSlotTypeDisplayName(SelectedSlotType)));
	}
	if (IsValid(SelectedSlotText))
	{
		SelectedSlotText->SetText(FText::Format(
			NSLOCTEXT("PBBumperEquipUI", "SelectedSlotFormat", "선택 위치 · {0}"),
			PBBumperEquipUIBuilder::GetBumperEquipSlotDisplayName(GetSelectedBumperEquipSlot())));
	}
	if (IsValid(DetailAccentBorder))
	{
		DetailAccentBorder->SetBrushColor(GetSlotColor(SelectedSlotType));
	}
}

void UPBBumperEquipUI::RefreshLoadoutPresentation()
{
	const int32 EquippedCount = IsValid(EquipController)
		? EquipController->GetEquippedBumperCount()
		: 0;

	if (IsValid(LoadoutStatusText))
	{
		LoadoutStatusText->SetText(FText::Format(
			NSLOCTEXT("PBBumperEquipUI", "LoadoutStatusFormat", "{0}개 중 {1}개 장착"),
			FText::AsNumber(BumperEquipSlotCount),
			FText::AsNumber(EquippedCount)));
	}

	if (!IsValid(SlotEquipStatusText))
	{
		return;
	}

	FName EquippedRowName = NAME_None;
	const FPBBumperTableRow* EquippedRow = nullptr;
	const EPBBumperEquipSlot SelectedEquipSlot = GetSelectedBumperEquipSlot();
	if (GetEquippedBumperForEquipSlot(SelectedEquipSlot, EquippedRowName) && IsValid(EquipController))
	{
		EquippedRow = EquipController->FindBumperRow(EquippedRowName);
	}

	SlotEquipStatusText->SetText(EquippedRow
		? FText::Format(
			NSLOCTEXT("PBBumperEquipUI", "EquippedBumperFormat", "장착됨 · {0}"),
			EquippedRow->DisplayName)
		: NSLOCTEXT("PBBumperEquipUI", "EmptyEquipSlot", "비어 있음"));
}

void UPBBumperEquipUI::ClearDetailPresentation()
{
	if (IsValid(DetailIconImage))
	{
		DetailIconImage->SetBrushFromTexture(nullptr);
		DetailIconImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.16f));
	}
	if (IsValid(DetailNameText))
	{
		DetailNameText->SetText(NSLOCTEXT("PBBumperEquipUI", "EmptyDetailName", "범퍼를 선택해 주세요"));
	}
	if (IsValid(DetailMetaText))
	{
		DetailMetaText->SetText(NSLOCTEXT("PBBumperEquipUI", "EmptyDetailMeta", "카탈로그에서 장착할 범퍼를 선택합니다."));
	}
	if (IsValid(DetailDescriptionText))
	{
		DetailDescriptionText->SetText(FText::GetEmpty());
	}
	if (IsValid(DetailTriggerText))
	{
		DetailTriggerText->SetText(NSLOCTEXT("PBBumperEquipUI", "EmptyTrigger", "발동 조건\n범퍼를 선택하면 조건을 확인할 수 있습니다."));
	}
	if (IsValid(DetailEffectText))
	{
		DetailEffectText->SetText(NSLOCTEXT("PBBumperEquipUI", "EmptyEffect", "효과\n범퍼를 선택하면 효과를 확인할 수 있습니다."));
	}
	if (IsValid(EquipActionLabel))
	{
		EquipActionLabel->SetText(NSLOCTEXT("PBBumperEquipUI", "EquipAction", "장착"));
	}
	if (IsValid(EquipActionButton))
	{
		EquipActionButton->SetIsEnabled(false);
		EquipActionButton->SetBackgroundColor(GetSlotColor(GetSelectedBumperSlotType()));
	}
}

void UPBBumperEquipUI::UpdateDetailPresentation(const FName RowName)
{
	const FPBBumperTableRow* Row = IsValid(EquipController)
		? EquipController->FindBumperRow(RowName)
		: nullptr;
	if (!Row)
	{
		ClearDetailPresentation();
		return;
	}

	if (IsValid(DetailIconImage))
	{
		DetailIconImage->SetBrushFromTexture(EquipController->ResolveBumperIconTexture(RowName, *Row));
		DetailIconImage->SetColorAndOpacity(FLinearColor::White);
	}
	if (IsValid(DetailNameText))
	{
		DetailNameText->SetText(Row->DisplayName);
	}
	if (IsValid(DetailMetaText))
	{
		DetailMetaText->SetText(FText::Format(
			NSLOCTEXT("PBBumperEquipUI", "DetailMetaFormat", "{0} · {1}"),
			PBBumperEquipUIBuilder::GetBumperRoleDisplayName(Row->RoleType),
			PBBumperEquipUIBuilder::GetBumperEffectTypeDisplayName(Row->EffectType)));
	}
	if (IsValid(DetailDescriptionText))
	{
		DetailDescriptionText->SetText(PBTextFormatUtils::FormatSingleValueTemplate(
			Row->Description,
			FText::AsNumber(Row->RequiredTriggerCount)));
	}

	const FText TriggerDescription = EquipController->ResolveBumperTriggerDescription(*Row);
	if (IsValid(DetailTriggerText))
	{
		DetailTriggerText->SetText(FText::Format(
			NSLOCTEXT("PBBumperEquipUI", "DetailTriggerFormat", "발동 조건\n{0}"),
			TriggerDescription.IsEmptyOrWhitespace()
				? NSLOCTEXT("PBBumperEquipUI", "MissingTrigger", "조건 정보가 없습니다.")
				: TriggerDescription));
	}

	const FText EffectDescription = EquipController->ResolveBumperEffectDescription(*Row);
	if (IsValid(DetailEffectText))
	{
		DetailEffectText->SetText(FText::Format(
			NSLOCTEXT("PBBumperEquipUI", "DetailEffectFormat", "효과\n{0}"),
			EffectDescription.IsEmptyOrWhitespace()
				? NSLOCTEXT("PBBumperEquipUI", "MissingEffect", "효과 정보가 없습니다.")
				: EffectDescription));
	}

	const bool bIsEquippedInCurrentSlot = IsSelectedBumperEquippedInCurrentSlot();
	const bool bIsEquippedInAnotherSlot = IsValid(EquipController)
		&& EquipController->IsBumperEquippedInAnotherSlot(RowName, GetSelectedBumperEquipSlot());
	if (IsValid(EquipActionLabel))
	{
		EquipActionLabel->SetText(bIsEquippedInCurrentSlot
			? NSLOCTEXT("PBBumperEquipUI", "UnequipAction", "해제")
			: bIsEquippedInAnotherSlot
				? NSLOCTEXT("PBBumperEquipUI", "EquippedElsewhereAction", "다른 위치에 장착됨")
				: NSLOCTEXT("PBBumperEquipUI", "EquipSelectedAction", "장착"));
	}
	if (IsValid(EquipActionButton))
	{
		// 이전 세션 데이터에 중복이 남아 있어도 현재 슬롯의 해제 동작은 막지 않는다.
		EquipActionButton->SetIsEnabled(bIsEquippedInCurrentSlot || !bIsEquippedInAnotherSlot);
		EquipActionButton->SetBackgroundColor(bIsEquippedInCurrentSlot
			? UnequipActionColor
			: GetSlotColor(GetSelectedBumperSlotType()));
	}
}

FLinearColor UPBBumperEquipUI::GetSlotColor(const EPBBumperSlotType SlotType) const
{
	switch (SlotType)
	{
	case EPBBumperSlotType::Top:
		return TopSlotColor;
	case EPBBumperSlotType::Side:
		return SideSlotColor;
	case EPBBumperSlotType::Rebound:
		return ReboundSlotColor;
	case EPBBumperSlotType::Special:
		return SpecialSlotColor;
	default:
		return FLinearColor::White;
	}
}

void UPBBumperEquipUI::SetBoardSlotButtonState(
	UButton* Button,
	const EPBBumperEquipSlot EquipSlot) const
{
	if (!IsValid(Button))
	{
		return;
	}

	EPBBumperSlotType SlotType;
	if (!PBBumperEquipSlotUtils::TryGetSlotType(EquipSlot, SlotType))
	{
		return;
	}

	FName EquippedRowName = NAME_None;
	const bool bHasEquippedBumper =
		GetEquippedBumperForEquipSlot(EquipSlot, EquippedRowName) && !EquippedRowName.IsNone();
	const FPBBumperTableRow* EquippedRow = bHasEquippedBumper && IsValid(EquipController)
		? EquipController->FindBumperRow(EquippedRowName)
		: nullptr;

	const FText SlotDisplayName = PBBumperEquipUIBuilder::GetBumperEquipSlotDisplayName(EquipSlot);
	const FText EquippedDisplayName = EquippedRow
		? EquippedRow->DisplayName
		: bHasEquippedBumper
			? FText::FromName(EquippedRowName)
			: NSLOCTEXT("PBBumperEquipUI", "EmptyBoardSlot", "비어 있음");

	const bool bIsSelected = GetSelectedBumperEquipSlot() == EquipSlot;
	const bool bIsDropHovered = HoveredDropSlot.IsSet() && HoveredDropSlot.GetValue() == EquipSlot;
	const FLinearColor SlotColor = GetSlotColor(SlotType);
	FLinearColor BackgroundColor = InactiveCategoryColor
		+ (SlotColor - InactiveCategoryColor) * 0.16f;
	BackgroundColor.A = 1.0f;
	if (bIsDropHovered)
	{
		BackgroundColor = InactiveCategoryColor
			+ (SlotColor - InactiveCategoryColor) * 0.82f;
		BackgroundColor.A = 1.0f;
	}
	else if (bIsSelected)
	{
		BackgroundColor = SlotColor;
	}
	else if (bHasEquippedBumper)
	{
		BackgroundColor = InactiveCategoryColor
			+ (SlotColor - InactiveCategoryColor) * EquippedSlotTintStrength;
		BackgroundColor.A = 1.0f;
	}
	Button->SetBackgroundColor(BackgroundColor);
	Button->SetToolTipText(FText::Format(
		NSLOCTEXT("PBBumperEquipUI", "BoardSlotTooltipFormat", "{0}: {1}"),
		SlotDisplayName,
		EquippedDisplayName));

	if (const TWeakObjectPtr<UImage>* IconReference = BoardSlotIconImages.Find(EquipSlot))
	{
		if (UImage* IconImage = IconReference->Get())
		{
			UTexture2D* IconTexture = EquippedRow
				? EquipController->ResolveBumperIconTexture(EquippedRowName, *EquippedRow)
				: nullptr;
			IconImage->SetBrushFromTexture(IconTexture);
			IconImage->SetVisibility(IsValid(IconTexture)
				? ESlateVisibility::HitTestInvisible
				: ESlateVisibility::Hidden);
		}
	}

	if (const TWeakObjectPtr<UTextBlock>* LabelReference = BoardSlotLabels.Find(EquipSlot))
	{
		if (UTextBlock* Label = LabelReference->Get())
		{
			Label->SetText(bIsDropHovered
				? FText::Format(
					NSLOCTEXT("PBBumperEquipUI", "BoardSlotDropLabelFormat", "{0}\n여기에 장착"),
					SlotDisplayName)
				: FText::Format(
					NSLOCTEXT("PBBumperEquipUI", "BoardSlotLabelFormat", "{0}\n{1}"),
					SlotDisplayName,
					EquippedDisplayName));

			FLinearColor LabelColor = SlotColor;
			LabelColor.A = UnselectedSlotOpacity;
			if (bIsDropHovered || bIsSelected || bHasEquippedBumper)
			{
				LabelColor = FLinearColor::White;
			}
			Label->SetColorAndOpacity(FSlateColor(LabelColor));
		}
	}
}

bool UPBBumperEquipUI::CanEquipBumperRowAtSlot(
	const FName RowName,
	const EPBBumperEquipSlot EquipSlot) const
{
	return IsValid(EquipController)
		&& EquipController->CanEquipBumperRowAtSlot(RowName, EquipSlot);
}

bool UPBBumperEquipUI::FindBoardSlotAtScreenPosition(
	const FVector2D& ScreenPosition,
	EPBBumperEquipSlot& OutEquipSlot) const
{
	const auto IsUnderPointer = [&ScreenPosition](const UButton* Button)
	{
		return IsValid(Button) && Button->GetCachedGeometry().IsUnderLocation(ScreenPosition);
	};

	if (IsUnderPointer(TopLeftMarker))
	{
		OutEquipSlot = EPBBumperEquipSlot::TopLeft;
		return true;
	}
	if (IsUnderPointer(TopRightMarker))
	{
		OutEquipSlot = EPBBumperEquipSlot::TopRight;
		return true;
	}
	if (IsUnderPointer(SideLeftMarker))
	{
		OutEquipSlot = EPBBumperEquipSlot::SideLeft;
		return true;
	}
	if (IsUnderPointer(SideRightMarker))
	{
		OutEquipSlot = EPBBumperEquipSlot::SideRight;
		return true;
	}
	if (IsUnderPointer(ReboundLeftMarker))
	{
		OutEquipSlot = EPBBumperEquipSlot::ReboundLeft;
		return true;
	}
	if (IsUnderPointer(ReboundRightMarker))
	{
		OutEquipSlot = EPBBumperEquipSlot::ReboundRight;
		return true;
	}
	if (IsUnderPointer(SpecialCenterMarker))
	{
		OutEquipSlot = EPBBumperEquipSlot::Special;
		return true;
	}

	return false;
}

void UPBBumperEquipUI::SetHoveredDropSlot(const TOptional<EPBBumperEquipSlot> EquipSlot)
{
	const bool bUnchanged = HoveredDropSlot.IsSet() == EquipSlot.IsSet()
		&& (!EquipSlot.IsSet() || HoveredDropSlot.GetValue() == EquipSlot.GetValue());
	if (bUnchanged)
	{
		return;
	}

	HoveredDropSlot = EquipSlot;
	RefreshBoardSlotSelection();
}

void UPBBumperEquipUI::SetCategoryButtonState(
	UButton* Button,
	const EPBBumperSlotType SlotType) const
{
	if (!IsValid(Button))
	{
		return;
	}

	const bool bIsSelected = GetSelectedBumperSlotType() == SlotType;
	Button->SetBackgroundColor(bIsSelected ? GetSlotColor(SlotType) : InactiveCategoryColor);

	if (UTextBlock* Label = Cast<UTextBlock>(Button->GetContent()))
	{
		Label->SetColorAndOpacity(FSlateColor(
			bIsSelected ? FLinearColor::White : InactiveControlTextColor));
	}
}

void UPBBumperEquipUI::BroadcastSelectedSlotChanged()
{
	OnSelectedBumperEquipSlotChanged.Broadcast(GetSelectedBumperEquipSlot());
	OnSelectedBumperSlotChanged.Broadcast(GetSelectedBumperSlotType());
}

void UPBBumperEquipUI::HandleTopLeftSlotClicked()
{
	SelectBumperEquipSlot(EPBBumperEquipSlot::TopLeft);
}

void UPBBumperEquipUI::HandleTopRightSlotClicked()
{
	SelectBumperEquipSlot(EPBBumperEquipSlot::TopRight);
}

void UPBBumperEquipUI::HandleSideLeftSlotClicked()
{
	SelectBumperEquipSlot(EPBBumperEquipSlot::SideLeft);
}

void UPBBumperEquipUI::HandleSideRightSlotClicked()
{
	SelectBumperEquipSlot(EPBBumperEquipSlot::SideRight);
}

void UPBBumperEquipUI::HandleReboundLeftSlotClicked()
{
	SelectBumperEquipSlot(EPBBumperEquipSlot::ReboundLeft);
}

void UPBBumperEquipUI::HandleReboundRightSlotClicked()
{
	SelectBumperEquipSlot(EPBBumperEquipSlot::ReboundRight);
}

void UPBBumperEquipUI::HandleSpecialSlotClicked()
{
	SelectBumperEquipSlot(EPBBumperEquipSlot::Special);
}

void UPBBumperEquipUI::HandleTopCategoryClicked()
{
	SelectBumperSlot(EPBBumperSlotType::Top);
}

void UPBBumperEquipUI::HandleSideCategoryClicked()
{
	SelectBumperSlot(EPBBumperSlotType::Side);
}

void UPBBumperEquipUI::HandleReboundCategoryClicked()
{
	SelectBumperSlot(EPBBumperSlotType::Rebound);
}

void UPBBumperEquipUI::HandleSpecialCategoryClicked()
{
	SelectBumperSlot(EPBBumperSlotType::Special);
}

void UPBBumperEquipUI::HandleEquipActionClicked()
{
	if (GetSelectedBumperRowName().IsNone())
	{
		return;
	}

	if (IsSelectedBumperEquippedInCurrentSlot())
	{
		UnequipSelectedBumper();
	}
	else
	{
		EquipSelectedBumper();
	}
}

bool UPBBumperEquipUI::IsSelectedBumperEquippedInCurrentSlot() const
{
	return IsValid(EquipController)
		&& EquipController->IsSelectedBumperEquippedInCurrentSlot();
}
