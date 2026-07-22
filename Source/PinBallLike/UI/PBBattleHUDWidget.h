#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/UI/Loading/PBLoadingScreenController.h"
#include "TimerManager.h"
#include "PBBattleHUDWidget.generated.h"

class UPBDeckOverviewWidget;
class APBBattleGameState;
class APBCombatPartyController;
class APBBallBase;
class UPBBallDeckSubsystem;
class UPBBallStatusWidget;
class UPanelWidget;
class UTextBlock;
class UTexture2D;
class AActor;
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
	void UnbindDisplayedBallEvents();
	void CacheDeckSubsystem();
	void CachePartyController();
	void BindDeckEvents();
	void UnbindDeckEvents();
	void EnsureDeckOverviewWidget();
	void ApplyBattlePhaseToDeckOverview(EPBBattleLevelPhase NewPhase);
	void ApplyBattlePhaseToLoadingScreen(EPBBattleLevelPhase NewPhase);
	void RegisterBattleMessageListeners();
	void UnregisterBattleMessageListeners();
	void CacheBattleGameState();
	void BindComboEvents();
	void UnbindComboEvents();
	void ScheduleBindComboEvents();
	void RefreshComboText();
	void ApplyComboText(int32 CurrentCombo);
	void ScheduleRefreshBallPanels();
	void RefreshDeckOverview();
	APBBallBase* FindPartyBallForDeploymentSlot(int32 SlotIndex) const;
	void SetBallPanel(int32 PanelIndex, APBBallBase* Ball);
	UPBBallStatusWidget* GetBallPanel(int32 PanelIndex) const;
	UTexture2D* GetBallIcon(APBBallBase* Ball) const;

	UFUNCTION()
	void HandleDeploymentSlotChanged(int32 SlotIndex, int32 BallInstanceId);

	UFUNCTION()
	void HandleDeploymentChanged();

	UFUNCTION()
	void HandleDisplayedBallDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void HandleBattleComboChanged(int32 CurrentCombo);

	void HandleBattlePhaseChangedMessage(FGameplayTag Channel, const FPBBattlePhaseChangedMessage& Message);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBDeckOverviewWidget> DeckOverviewWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> BallPanelContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Combo;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBallStatusWidget>> BallPanels;

	TArray<TWeakObjectPtr<APBBallBase>> DisplayedBalls;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDeckSubsystem> DeckSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<APBCombatPartyController> PartyController;

	UPROPERTY(Transient)
	TObjectPtr<APBBattleGameState> BattleGameState;

	FGameplayMessageListenerHandle BattlePhaseChangedListenerHandle;
	TUniquePtr<FPBLoadingScreenController> LoadingScreenController;

	FTimerHandle ComboBindRetryTimerHandle;

	bool bDeckEventsBound = false;
	bool bComboEventsBound = false;
};
