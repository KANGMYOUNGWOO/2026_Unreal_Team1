// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBDeckWidget.generated.h"

class UPBBallDeckSubsystem;
class UPBBallSlotWidget;
class UUniformGridPanel;
class UButton;
/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBDeckWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
private:
	void BuildBenchSlots();
	void BindDeckEvents();
	void UnbindDeckEvents();
	void RefreshBenchSlots();

	UFUNCTION()
	void HandleBenchSlotChanged(int32 SlotIndex, int32 BallInstanceId);

	UFUNCTION()
	void HandleBenchSlotsSwapped();

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UButton> Button_Exit;
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UUniformGridPanel> GridPanel_BallList;
	
	UPROPERTY(EditDefaultsOnly, Category = "BallDeck", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPBBallSlotWidget> WBP_BallSlotClass;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDeckSubsystem> DeckSubsystem;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBallSlotWidget>> BenchSlotWidgets;
};
