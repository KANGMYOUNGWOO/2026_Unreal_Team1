#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Synergy/PBSynergyState.h"
#include "PinBallLike/Struct/Synergy/PBSynergyViewData.h"
#include "PBDeckOverviewWidget.generated.h"

class UPBBallDeckSubsystem;
class UPBBallDeckSynergyService;
class UPBDeckOverviewViewModel;
class UPBDeckWidget;
class UPBDeploymentWidget;
class UPBUIManagerSubsystem;
class UButton;

UCLASS()
class PINBALLLIKE_API UPBDeckOverviewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DeckOverview|Deck")
	void RefreshDeck();

	UFUNCTION(BlueprintCallable, Category = "DeckOverview|Deployment")
	void RefreshDeployment();

	UFUNCTION(BlueprintCallable, Category = "DeckOverview|Synergy")
	void RefreshSynergyPanels();

	UFUNCTION(BlueprintCallable, Category = "DeckOverview")
	void RefreshAll();

	UFUNCTION(BlueprintPure, Category = "DeckOverview|ViewModel")
	UPBDeckOverviewViewModel* GetViewModel() const { return ViewModel; }

	void ShutdownOverview();

	UFUNCTION(BlueprintCallable, Category = "DeckOverview|Animation")
	void OpenDeck();

	UFUNCTION(BlueprintCallable, Category = "DeckOverview|Animation")
	void CloseDeck();

	UFUNCTION(BlueprintCallable, Category = "DeckOverview|Animation")
	void ToggleDeck();

	UFUNCTION(BlueprintCallable, Category = "DeckOverview|Animation")
	void OpenDeployment();

	UFUNCTION(BlueprintCallable, Category = "DeckOverview|Animation")
	void CloseDeployment();

	UFUNCTION(BlueprintCallable, Category = "DeckOverview|Animation")
	void OpenAll();

	UFUNCTION(BlueprintCallable, Category = "DeckOverview|Animation")
	void CloseAll();

	UFUNCTION(BlueprintCallable, Category = "DeckOverview|Animation")
	void NotifyDeckAnimationFinished();

	UFUNCTION(BlueprintCallable, Category = "DeckOverview|Animation")
	void NotifyDeploymentAnimationFinished();

	UFUNCTION(BlueprintPure, Category = "DeckOverview|Animation")
	bool IsDeckOpen() const { return bIsDeckOpen; }

	UFUNCTION(BlueprintPure, Category = "DeckOverview|Animation")
	bool IsDeploymentOpen() const { return bIsDeploymentOpen; }

	UFUNCTION(BlueprintPure, Category = "DeckOverview|Animation")
	bool IsDeckAnimationPlaying() const { return bIsDeckAnimationPlaying; }

	UFUNCTION(BlueprintPure, Category = "DeckOverview|Animation")
	bool IsDeploymentAnimationPlaying() const { return bIsDeploymentAnimationPlaying; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "DeckOverview|Animation")
	void BP_PlayDeckOpenAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "DeckOverview|Animation")
	void BP_PlayDeckCloseAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "DeckOverview|Animation")
	void BP_PlayDeploymentOpenAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "DeckOverview|Animation")
	void BP_PlayDeploymentCloseAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "DeckOverview|Synergy")
	void BP_OnSynergyViewDataChanged(const TArray<FPBSynergyViewData>& ActiveSynergyViewData);

	UPROPERTY(BlueprintReadOnly, Category = "DeckOverview|Animation")
	bool bIsDeckOpen = false;

	UPROPERTY(BlueprintReadOnly, Category = "DeckOverview|Animation")
	bool bIsDeploymentOpen = false;

	UPROPERTY(BlueprintReadOnly, Category = "DeckOverview|Animation")
	bool bIsDeckAnimationPlaying = false;

	UPROPERTY(BlueprintReadOnly, Category = "DeckOverview|Animation")
	bool bIsDeploymentAnimationPlaying = false;

private:
	void CacheDeckSubsystem();
	void BindDeckEvents();
	void UnbindDeckEvents();
	UPBBallDeckSynergyService* GetSynergyService() const;
	void BindSynergyEvents();
	void UnbindSynergyEvents();
	void ScheduleRefreshSynergyPanels();
	void EnsureViewModel();
	bool ApplyViewModelToWidget();
	void BindGlobalToolbarEvents();
	void UnbindGlobalToolbarEvents();

	UFUNCTION()
	void HandleGlobalDeckToggleRequested();

	UFUNCTION()
	void HandleDeploymentSlotChanged(int32 SlotIndex, int32 BallInstanceId);

	UFUNCTION()
	void HandleDeploymentChanged();

	void HandleSynergyStatesChanged(const TArray<FPBSynergyState>& SynergyStates);

	void InitializeOverview();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBDeckWidget> DeckWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBDeploymentWidget> DeploymentWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Deck;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDeckSubsystem> DeckSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<UPBDeckOverviewViewModel> ViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UPBUIManagerSubsystem> UIManagerSubsystem;

	FDelegateHandle SynergyStatesChangedHandle;

	bool bDeckEventsBound = false;
	bool bSynergyEventsBound = false;
};
