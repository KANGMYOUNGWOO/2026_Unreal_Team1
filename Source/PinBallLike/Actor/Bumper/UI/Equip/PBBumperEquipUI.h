#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/UI/PBUserWidget.h"
#include "PBBumperEquipUI.generated.h"

class UPBBumperInfoPanelViewModel;
class UPBBumperListItemObject;
class UPBGameDataLoadSubsystem;
class UPBPlayerDataSubsystem;
class UPBTableDataSubsystem;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBBumperListItemsReadySignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPBBumperSelectedSlotChangedSignature,
	EPBBumperSlotType,
	SelectedSlotType);

UCLASS(BlueprintType, Blueprintable)
class PINBALLLIKE_API UPBBumperEquipUI : public UPBUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
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

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	void SelectBumperRow(FName RowName);

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool EquipBumperRow(FName RowName);

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool UnequipBumperRow(FName RowName);

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool EquipSelectedBumper();

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool UnequipSelectedBumper();

	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	FName GetSelectedBumperRowName() const { return SelectedBumperRowName; }

	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	EPBBumperSlotType GetSelectedBumperSlotType() const { return SelectedBumperSlotType; }

	/** 보드 미리보기에 표시할 슬롯별 현재 장착 Row를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	bool GetEquippedBumperForSlot(EPBBumperSlotType SlotType, FName& OutBumperRowId) const;

	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	UPBBumperInfoPanelViewModel* GetInfoPanelViewModel() const { return InfoPanelViewModel; }

	UPROPERTY(BlueprintAssignable, Category = "Bumper|EquipUI")
	FPBBumperListItemsReadySignature OnBumperListItemsReady;

	UPROPERTY(BlueprintAssignable, Category = "Bumper|EquipUI")
	FPBBumperSelectedSlotChangedSignature OnSelectedBumperSlotChanged;

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
	void RefreshBoardSlotSelection();
	void SetBoardSlotButtonState(UButton* Button, EPBBumperSlotType SlotType) const;

	UFUNCTION()
	void HandleTopSlotClicked();

	UFUNCTION()
	void HandleSideSlotClicked();

	UFUNCTION()
	void HandleReboundSlotClicked();

	UFUNCTION()
	void HandleSpecialSlotClicked();

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
	EPBBumperSlotType SelectedBumperSlotType = EPBBumperSlotType::Top;

	bool bBumperRowsLoaded = false;
	bool bBumperUIAssetLoadPending = false;
	bool bBumperListItemObjectsBuilt = false;
};
