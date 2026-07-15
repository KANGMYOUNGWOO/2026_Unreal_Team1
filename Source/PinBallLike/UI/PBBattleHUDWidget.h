#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Struct/Synergy/PBSynergyState.h"
#include "PBBattleHUDWidget.generated.h"

class APBCombatPartyController;
class APBBallBase;
struct FPBBattlePhaseChangedMessage;
class UPBBallDeckSubsystem;
class UPBBallDeckSynergyService;
class UPBBallStatusWidget;
class UPBBattleHUDViewModel;
class UPBSynergyPanelWidget;
class UPanelWidget;
class UTexture2D;

UCLASS()
class PINBALLLIKE_API UPBBattleHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BattleHUD|Ball")
	void RefreshBallPanels();

	UFUNCTION(BlueprintCallable, Category = "BattleHUD|Synergy")
	void RefreshSynergyPanels();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BattleHUD|Synergy")
	TSubclassOf<UPBSynergyPanelWidget> SynergyPanelWidgetClass;

private:
	static constexpr int32 MaxBallPanelCount = 3;

	void CacheBallPanels();
	void CacheDeckSubsystem();
	void CachePartyController();
	void BindDeckEvents();
	void UnbindDeckEvents();
	UPBBallDeckSynergyService* GetSynergyService() const;
	void BindSynergyEvents();
	void UnbindSynergyEvents();
	void RegisterBattleMessageListeners();
	void UnregisterBattleMessageListeners();
	void ScheduleRefreshBallPanels();
	void ScheduleRefreshSynergyPanels();
	void EnsureBattleHUDViewModel();
	void SetBallPanel(int32 PanelIndex, APBBallBase* Ball);
	UPBBallStatusWidget* GetBallPanel(int32 PanelIndex) const;
	UTexture2D* GetBallIcon(APBBallBase* Ball) const;

	UFUNCTION()
	void HandleDeploymentSlotChanged(int32 SlotIndex, int32 BallInstanceId);

	UFUNCTION()
	void HandleDeploymentChanged();

	void HandleSynergyStatesChanged(const TArray<FPBSynergyState>& SynergyStates);
	void HandleBattlePhaseChangedMessage(FGameplayTag Channel, const FPBBattlePhaseChangedMessage& Message);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> BallPanelContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> SynergyPanelContainer;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBallStatusWidget>> BallPanels;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDeckSubsystem> DeckSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<APBCombatPartyController> PartyController;

	UPROPERTY(Transient)
	TObjectPtr<UPBBattleHUDViewModel> BattleHUDViewModel;

	FGameplayMessageListenerHandle BattlePhaseChangedListenerHandle;
	FDelegateHandle SynergyStatesChangedHandle;

	bool bDeckEventsBound = false;
	bool bSynergyEventsBound = false;
};
