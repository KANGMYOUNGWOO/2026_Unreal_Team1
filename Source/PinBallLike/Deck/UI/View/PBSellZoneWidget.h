// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PBSellZoneWidget.generated.h"

struct FPBDeckDragEndedMessage;
struct FPBDeckDragStartedMessage;
class UPBSellZoneViewModel;
class UDragDropOperation;

/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBSellZoneWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BallDeck")
	void SetSellEnabled(bool bInSellEnabled);

	UFUNCTION(BlueprintCallable, Category = "BallDeck")
	void SetSellBallInstanceId(int32 InBallInstanceId);

	UFUNCTION(BlueprintCallable, Category = "BallDeck")
	void ClearSellBall();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	void HandleBallItemDragStarted(FGameplayTag Channel, const FPBDeckDragStartedMessage& Message);
	void HandleBallItemDragEnded(FGameplayTag Channel, const FPBDeckDragEndedMessage& Message);
	void EnsureSellZoneViewModel();
	bool ApplyViewModelToWidget();
	
	UPROPERTY(Transient)
	TObjectPtr<UPBSellZoneViewModel> SellZoneViewModel;

	FGameplayMessageListenerHandle DragStartedHandle;
	FGameplayMessageListenerHandle DragEndedHandle;

	bool bSellEnabled = false;
};
