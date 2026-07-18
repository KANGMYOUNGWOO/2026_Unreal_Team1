#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/UI/PBUserWidget.h"
#include "PBBumperEquipUI.generated.h"

class UPBBumperInfoPanelViewModel;
class UPBBumperListItemObject;
class UPBBumperDragDropOperation;
class UPBGameDataLoadSubsystem;
class UPBPlayerDataSubsystem;
class UPBTableDataSubsystem;
class UDragDropOperation;
class UBorder;
class UButton;
class UImage;
class UTextBlock;
class UWidgetSwitcher;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBBumperListItemsReadySignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPBBumperSelectedSlotChangedSignature,
	EPBBumperSlotType,
	SelectedSlotType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPBBumperSelectedEquipSlotChangedSignature,
	EPBBumperEquipSlot,
	SelectedEquipSlot);

/**
 * 범퍼 카탈로그 조회와 7개 물리 슬롯의 장착·해제를 조정하는 메인 UI입니다.
 * DataTable과 PlayerDataSubsystem을 원본으로 사용하며, 버튼과 드래그 입력을 같은 장착 API로 연결합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PINBALLLIKE_API UPBBumperEquipUI : public UPBUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	void GetBumperListItemObjects(
		TArray<UPBBumperListItemObject*>& OutTopItems,
		TArray<UPBBumperListItemObject*>& OutSideItems,
		TArray<UPBBumperListItemObject*>& OutReboundItems,
		TArray<UPBBumperListItemObject*>& OutSpecialItems) const;

	/** 보드 미리보기와 목록이 함께 사용할 현재 장착 슬롯을 선택한다. */
	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool SelectBumperSlot(EPBBumperSlotType SlotType);

	/** 좌우를 포함한 실제 보드 장착 위치를 선택합니다. */
	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool SelectBumperEquipSlot(EPBBumperEquipSlot EquipSlot);

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	void SelectBumperRow(FName RowName);

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool EquipBumperRow(FName RowName);

	/** 드롭 대상까지 명시하여 범퍼를 장착합니다. 카테고리가 맞지 않으면 상태를 바꾸지 않습니다. */
	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool EquipBumperRowAtSlot(FName RowName, EPBBumperEquipSlot EquipSlot);

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool UnequipBumperRow(FName RowName);

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool EquipSelectedBumper();

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool UnequipSelectedBumper();

	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	FName GetSelectedBumperRowName() const { return SelectedBumperRowName; }

	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	EPBBumperSlotType GetSelectedBumperSlotType() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	EPBBumperEquipSlot GetSelectedBumperEquipSlot() const { return SelectedBumperEquipSlot; }

	/** 보드 미리보기에 표시할 슬롯별 현재 장착 Row를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	bool GetEquippedBumperForSlot(EPBBumperSlotType SlotType, FName& OutBumperRowId) const;

	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	bool GetEquippedBumperForEquipSlot(EPBBumperEquipSlot EquipSlot, FName& OutBumperRowId) const;

	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	UPBBumperInfoPanelViewModel* GetInfoPanelViewModel() const { return InfoPanelViewModel; }

	UPROPERTY(BlueprintAssignable, Category = "Bumper|EquipUI")
	FPBBumperListItemsReadySignature OnBumperListItemsReady;

	UPROPERTY(BlueprintAssignable, Category = "Bumper|EquipUI")
	FPBBumperSelectedSlotChangedSignature OnSelectedBumperSlotChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bumper|EquipUI")
	FPBBumperSelectedEquipSlotChangedSignature OnSelectedBumperEquipSlotChanged;

protected:
	virtual void NativeOnDragLeave(
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

private:
	UFUNCTION()
	void HandleStartupGameDataLoaded();

	UFUNCTION()
	void HandleBumperUIAssetsLoaded();

	void CacheRequiredSubsystems();
	void BindDataLoadEvents();
	void UnbindDataLoadEvents();
	void EnsureInfoPanelViewModel();
	bool LoadBumperRowsOnce();
	void RequestBumperUIAssetsAsync();
	void BuildBumperListItemObjects();
	void UpdateInfoPanelByRowName(FName RowName);
	void UpdateBumperListEquipStates();
	void RefreshBumperEquipState(FName RowName);
	void BindBoardSlotButtons();
	void BuildBoardSlotPresentations();
	void BuildBoardSlotPresentation(UButton* Button, EPBBumperEquipSlot EquipSlot);
	void BindRedesignedControls();
	void RefreshBoardSlotSelection();
	void RefreshRedesignedPresentation();
	void RefreshCategoryPresentation();
	void RefreshLoadoutPresentation();
	void ClearDetailPresentation();
	void UpdateDetailPresentation(FName RowName);
	FLinearColor GetSlotColor(EPBBumperSlotType SlotType) const;
	void SetBoardSlotButtonState(UButton* Button, EPBBumperEquipSlot EquipSlot) const;
	void SetCategoryButtonState(UButton* Button, EPBBumperSlotType SlotType) const;
	bool CanEquipBumperRowAtSlot(FName RowName, EPBBumperEquipSlot EquipSlot) const;
	bool FindBoardSlotAtScreenPosition(const FVector2D& ScreenPosition, EPBBumperEquipSlot& OutEquipSlot) const;
	void SetHoveredDropSlot(TOptional<EPBBumperEquipSlot> EquipSlot);
	void BroadcastSelectedSlotChanged();
	bool IsSelectedBumperEquippedInCurrentSlot() const;

	UFUNCTION()
	void HandleTopLeftSlotClicked();

	UFUNCTION()
	void HandleTopRightSlotClicked();

	UFUNCTION()
	void HandleSideLeftSlotClicked();

	UFUNCTION()
	void HandleSideRightSlotClicked();

	UFUNCTION()
	void HandleReboundLeftSlotClicked();

	UFUNCTION()
	void HandleReboundRightSlotClicked();

	UFUNCTION()
	void HandleSpecialSlotClicked();

	UFUNCTION()
	void HandleTopCategoryClicked();

	UFUNCTION()
	void HandleSideCategoryClicked();

	UFUNCTION()
	void HandleReboundCategoryClicked();

	UFUNCTION()
	void HandleSpecialCategoryClicked();

	UFUNCTION()
	void HandleEquipActionClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> TopLeftMarker;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> TopRightMarker;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SideLeftMarker;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SideRightMarker;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ReboundLeftMarker;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ReboundRightMarker;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SpecialCenterMarker;

	/** 새 화면은 네 카테고리 목록을 동시에 늘어놓지 않고 선택한 위치의 목록만 보여줍니다. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> CatalogSwitcher;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> TopCategoryButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SideCategoryButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ReboundCategoryButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SpecialCategoryButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CatalogTitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SelectedSlotText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LoadoutStatusText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SlotEquipStatusText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> DetailAccentBorder;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> DetailIconImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailMetaText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailDescriptionText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailTriggerText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailEffectText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> EquipActionButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EquipActionLabel;

	/** 보드 미리보기의 슬롯 계열별 기본색입니다. Widget Blueprint 기본값에서 조정할 수 있습니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true"))
	FLinearColor TopSlotColor = FLinearColor(0.85f, 0.27f, 0.24f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true"))
	FLinearColor SideSlotColor = FLinearColor(0.18f, 0.55f, 0.78f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true"))
	FLinearColor ReboundSlotColor = FLinearColor(0.93f, 0.56f, 0.16f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true"))
	FLinearColor SpecialSlotColor = FLinearColor(0.22f, 0.67f, 0.49f, 1.0f);

	/** 선택되지 않은 슬롯의 계열색 라벨에 적용할 투명도입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float UnselectedSlotOpacity = 0.82f;

	/** 장착됐지만 현재 선택되지 않은 배치 슬롯에 카테고리색을 섞는 비율입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float EquippedSlotTintStrength = 0.34f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true"))
	FLinearColor InactiveCategoryColor = FLinearColor(0.075f, 0.081f, 0.089f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true"))
	FLinearColor InactiveControlTextColor = FLinearColor(0.67f, 0.69f, 0.71f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true"))
	FLinearColor UnequipActionColor = FLinearColor(0.40f, 0.12f, 0.14f, 1.0f);

	UPROPERTY(Transient)
	TObjectPtr<UPBBumperInfoPanelViewModel> InfoPanelViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UPBTableDataSubsystem> TableDataSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<UPBPlayerDataSubsystem> PlayerDataSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<UPBGameDataLoadSubsystem> GameDataLoadSubsystem;

	UPROPERTY(Transient)
	TArray<FName> BumperRowNames;

	UPROPERTY(Transient)
	TArray<FPBBumperTableRow> BumperRows;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBumperListItemObject>> TopItems;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBumperListItemObject>> SideItems;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBumperListItemObject>> ReboundItems;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBumperListItemObject>> SpecialItems;

	UPROPERTY(Transient)
	FName SelectedBumperRowName = NAME_None;

	UPROPERTY(Transient)
	EPBBumperEquipSlot SelectedBumperEquipSlot = EPBBumperEquipSlot::TopLeft;

	/** 런타임에 기존 슬롯 버튼 안에 생성한 아이콘과 라벨입니다. WidgetTree가 수명을 소유합니다. */
	TMap<EPBBumperEquipSlot, TWeakObjectPtr<UImage>> BoardSlotIconImages;
	TMap<EPBBumperEquipSlot, TWeakObjectPtr<UTextBlock>> BoardSlotLabels;
	TOptional<EPBBumperEquipSlot> HoveredDropSlot;

	bool bBumperRowsLoaded = false;
	bool bBumperUIAssetLoadPending = false;
	bool bBumperListItemObjectsBuilt = false;
	bool bWidgetConstructed = false;
};
