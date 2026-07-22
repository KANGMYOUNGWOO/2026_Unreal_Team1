#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Struct/UI/PBDamageLogMessage.h"
#include "PBDamageLogWidget.generated.h"

class UPanelWidget;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBDamageLogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Damage Log")
	bool ShowDamage(EPBDamageLogStyle InLogType, int32 Damage, FVector WorldLocation);

	UFUNCTION(BlueprintCallable, Category = "UI|Damage Log")
	void ReleaseEntry(UUserWidget* Entry);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Damage Log")
	void OnEntryCreated(UUserWidget* Entry);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Damage Log")
	void OnEntryActivated(UUserWidget* Entry, EPBDamageLogStyle InLogType, int32 Damage, FVector WorldLocation);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> EntryContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Damage Log", meta = (ClampMin = "1"))
	int32 PoolSize = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Damage Log")
	TSubclassOf<UUserWidget> EntryWidgetClass;

private:
	void HandleDamageLogRequested(FGameplayTag Channel, const FPBDamageLogMessage& Message);
	void InitializePool();
	UUserWidget* AcquireEntry();
	void ApplyEntryOutlineColor(UUserWidget* Entry, EPBDamageLogStyle InLogType) const;

	FGameplayMessageListenerHandle DamageLogRequestedHandle;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UUserWidget>> InactiveEntries;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UUserWidget>> ActiveEntries;
};
