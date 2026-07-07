#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/UI/PBUserWidget.h"
#include "PBBumperEquipUI.generated.h"

class UPBBumperInfoPanelViewModel;
class UPBBumperListItemObject;
class UPBGameDataLoadSubsystem;
class UPBPlayerDataSubsystem;
class UPBTableDataSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBBumperListItemsReadySignature);

UCLASS(BlueprintType, Blueprintable)
class PINBALLLIKE_API UPBBumperEquipUI : public UPBUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	void GetBumperListItemObjects(
		TArray<UPBBumperListItemObject*>& OutReboundItems,
		TArray<UPBBumperListItemObject*>& OutSideItems,
		TArray<UPBBumperListItemObject*>& OutTopTargetItems) const;

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
	UPBBumperInfoPanelViewModel* GetInfoPanelViewModel() const { return InfoPanelViewModel; }

	UPROPERTY(BlueprintAssignable, Category = "Bumper|EquipUI")
	FPBBumperListItemsReadySignature OnBumperListItemsReady;

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
	TArray<TObjectPtr<UPBBumperListItemObject>> ReboundItems;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBumperListItemObject>> SideItems;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBumperListItemObject>> TopTargetItems;

	UPROPERTY(Transient)
	FName SelectedBumperRowName = NAME_None;

	bool bBumperRowsLoaded = false;
	bool bBumperUIAssetLoadRequested = false;
	bool bBumperListItemObjectsBuilt = false;
};
