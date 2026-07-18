#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Bumper/PBBumperEquipSlot.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "UObject/Object.h"
#include "PBBumperEquipController.generated.h"

class UGameInstance;
class UTexture2D;
class UPBBumperInfoPanelViewModel;
class UPBBumperListItemObject;
class UPBGameDataLoadSubsystem;
class UPBPlayerDataSubsystem;
class UPBTableDataSubsystem;

DECLARE_MULTICAST_DELEGATE(FPBBumperEquipCatalogReadyNative);
DECLARE_MULTICAST_DELEGATE(FPBBumperEquipSelectionChangedNative);

/**
 * 범퍼 장착 화면의 데이터 로드, 선택 상태, 장착 명령을 조정합니다.
 * 화면 위젯이나 레이아웃에는 접근하지 않으며 PlayerData와 DataSubsystem만 원본으로 사용합니다.
 */
UCLASS()
class PINBALLLIKE_API UPBBumperEquipController : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UGameInstance* InGameInstance);
	void Shutdown();

	bool IsCatalogReady() const { return bBumperListItemObjectsBuilt; }
	void GetBumperListItemObjects(
		TArray<UPBBumperListItemObject*>& OutTopItems,
		TArray<UPBBumperListItemObject*>& OutSideItems,
		TArray<UPBBumperListItemObject*>& OutReboundItems,
		TArray<UPBBumperListItemObject*>& OutSpecialItems) const;

	bool SelectBumperSlot(EPBBumperSlotType SlotType);
	bool SelectBumperEquipSlot(EPBBumperEquipSlot EquipSlot);

	UFUNCTION()
	void SelectBumperRow(FName RowName);

	bool EquipBumperRow(FName RowName);
	bool EquipBumperRowAtSlot(FName RowName, EPBBumperEquipSlot EquipSlot);
	bool UnequipBumperRow(FName RowName);
	bool EquipSelectedBumper();
	bool UnequipSelectedBumper();

	FName GetSelectedBumperRowName() const { return SelectedBumperRowName; }
	EPBBumperSlotType GetSelectedBumperSlotType() const;
	EPBBumperEquipSlot GetSelectedBumperEquipSlot() const { return SelectedBumperEquipSlot; }
	bool GetEquippedBumperForSlot(EPBBumperSlotType SlotType, FName& OutBumperRowId) const;
	bool GetEquippedBumperForEquipSlot(EPBBumperEquipSlot EquipSlot, FName& OutBumperRowId) const;
	int32 GetEquippedBumperCount() const;

	const FPBBumperTableRow* FindBumperRow(FName RowName) const;
	UTexture2D* ResolveBumperIconTexture(FName RowName, const FPBBumperTableRow& Row) const;
	FText ResolveBumperTriggerDescription(const FPBBumperTableRow& Row) const;
	FText ResolveBumperEffectDescription(const FPBBumperTableRow& Row) const;
	bool CanEquipBumperRowAtSlot(FName RowName, EPBBumperEquipSlot EquipSlot) const;
	bool IsSelectedBumperEquippedInCurrentSlot() const;
	bool IsBumperEquippedInAnotherSlot(FName RowName, EPBBumperEquipSlot TargetSlot) const;
	UPBBumperInfoPanelViewModel* GetInfoPanelViewModel() const { return InfoPanelViewModel; }

	FPBBumperEquipCatalogReadyNative OnCatalogReady;
	FPBBumperEquipSelectionChangedNative OnSelectionChanged;

private:
	UFUNCTION()
	void HandleStartupGameDataLoaded();

	UFUNCTION()
	void HandleBumperUIAssetsLoaded();

	void CacheRequiredSubsystems(UGameInstance* GameInstance);
	void BindDataLoadEvents();
	void UnbindDataLoadEvents();
	void EnsureInfoPanelViewModel();
	bool LoadBumperRowsOnce();
	void RequestBumperUIAssetsAsync();
	void BuildBumperListItemObjects();
	void BindListItemSelectionEvents();
	void UpdateInfoPanelByRowName(FName RowName);
	void UpdateBumperListEquipStates();
	void RefreshBumperEquipState(FName RowName);

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

	bool bInitialized = false;
	bool bBumperRowsLoaded = false;
	bool bBumperUIAssetLoadPending = false;
	bool bBumperListItemObjectsBuilt = false;
};
