#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/UI/Loading/PBLoadingScreenController.h"
#include "PBBattleHUDWidget.generated.h"

class UPBDeckOverviewWidget;
class APBCombatPartyController;
class APBBallBase;
class UPBBallDeckSubsystem;
class UPBBallStatusWidget;
class UPanelWidget;
class UTexture2D;
enum class EPBBattleLevelPhase : uint8;
struct FPBBattlePhaseChangedMessage;

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

private:
	static constexpr int32 MaxBallPanelCount = 3;

	void CacheBallPanels();
	void CacheDeckSubsystem();
	void CachePartyController();
	void BindDeckEvents();
	void UnbindDeckEvents();
	void EnsureDeckOverviewWidget();
	void ApplyBattlePhaseToDeckOverview(EPBBattleLevelPhase NewPhase);
	void ApplyBattlePhaseToLoadingScreen(EPBBattleLevelPhase NewPhase);
	void RegisterBattleMessageListeners();
	void UnregisterBattleMessageListeners();
	void ScheduleRefreshBallPanels();
	void RefreshDeckOverview();
	void SetBallPanel(int32 PanelIndex, APBBallBase* Ball);
	UPBBallStatusWidget* GetBallPanel(int32 PanelIndex) const;
	UTexture2D* GetBallIcon(APBBallBase* Ball) const;

	UFUNCTION()
	void HandleDeploymentSlotChanged(int32 SlotIndex, int32 BallInstanceId);

	UFUNCTION()
	void HandleDeploymentChanged();

	void HandleBattlePhaseChangedMessage(FGameplayTag Channel, const FPBBattlePhaseChangedMessage& Message);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBDeckOverviewWidget> DeckOverviewWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> BallPanelContainer;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBallStatusWidget>> BallPanels;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDeckSubsystem> DeckSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<APBCombatPartyController> PartyController;

	FGameplayMessageListenerHandle BattlePhaseChangedListenerHandle;
	TUniquePtr<FPBLoadingScreenController> LoadingScreenController;

	bool bDeckEventsBound = false;
};
