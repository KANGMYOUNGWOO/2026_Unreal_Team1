#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Bumper/PBBumperEquipSlot.h"
#include "PinBallLike/UI/PBUserWidget.h"
#include "PBBumperEquipUI.generated.h"

class UPBBumperEquipController;
class UPBBumperInfoPanelViewModel;
class UPBBumperListItemObject;
class UPBBumperDragDropOperation;
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

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool SelectBumperSlot(EPBBumperSlotType SlotType);

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool SelectBumperEquipSlot(EPBBumperEquipSlot EquipSlot);

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	void SelectBumperRow(FName RowName);

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool EquipBumperRow(FName RowName);

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool EquipBumperRowAtSlot(FName RowName, EPBBumperEquipSlot EquipSlot);

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool UnequipBumperRow(FName RowName);

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool EquipSelectedBumper();

	UFUNCTION(BlueprintCallable, Category = "Bumper|EquipUI")
	bool UnequipSelectedBumper();

	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	FName GetSelectedBumperRowName() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	EPBBumperSlotType GetSelectedBumperSlotType() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	EPBBumperEquipSlot GetSelectedBumperEquipSlot() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	bool GetEquippedBumperForSlot(EPBBumperSlotType SlotType, FName& OutBumperRowId) const;

	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	bool GetEquippedBumperForEquipSlot(EPBBumperEquipSlot EquipSlot, FName& OutBumperRowId) const;

	UFUNCTION(BlueprintPure, Category = "Bumper|EquipUI")
	UPBBumperInfoPanelViewModel* GetInfoPanelViewModel() const;

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
	void EnsureEquipController();
	void BindControllerEvents();
	void UnbindControllerEvents();
	void HandleControllerCatalogReady();
	void HandleControllerSelectionChanged();
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true"))
	FLinearColor TopSlotColor = FLinearColor(0.85f, 0.27f, 0.24f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true"))
	FLinearColor SideSlotColor = FLinearColor(0.18f, 0.55f, 0.78f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true"))
	FLinearColor ReboundSlotColor = FLinearColor(0.93f, 0.56f, 0.16f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true"))
	FLinearColor SpecialSlotColor = FLinearColor(0.22f, 0.67f, 0.49f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float UnselectedSlotOpacity = 0.82f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float EquippedSlotTintStrength = 0.34f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true"))
	FLinearColor InactiveCategoryColor = FLinearColor(0.075f, 0.081f, 0.089f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true"))
	FLinearColor InactiveControlTextColor = FLinearColor(0.67f, 0.69f, 0.71f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|EquipUI|Style", meta = (AllowPrivateAccess = "true"))
	FLinearColor UnequipActionColor = FLinearColor(0.40f, 0.12f, 0.14f, 1.0f);

	UPROPERTY(Transient)
	TObjectPtr<UPBBumperEquipController> EquipController;

	TMap<EPBBumperEquipSlot, TWeakObjectPtr<UImage>> BoardSlotIconImages;
	TMap<EPBBumperEquipSlot, TWeakObjectPtr<UTextBlock>> BoardSlotLabels;
	TOptional<EPBBumperEquipSlot> HoveredDropSlot;

	bool bWidgetConstructed = false;
	bool bCatalogReadyBroadcastForConstruct = false;
};
