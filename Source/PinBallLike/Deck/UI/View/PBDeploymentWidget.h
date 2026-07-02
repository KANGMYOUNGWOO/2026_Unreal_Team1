// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBDeploymentWidget.generated.h"

class UPBBallDeckSubsystem;
class UPBBallSlotWidget;
class UUniformGridPanel;
/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBDeploymentWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BuildDeploymentSlots();
	void BindDeckEvents();
	void UnbindDeckEvents();
	void RefreshDeploymentSlots();

	UFUNCTION()
	void HandleDeploymentSlotChanged(int32 SlotIndex, int32 BallInstanceId);

	UFUNCTION()
	void HandleDeploymentSlotsReordered();

	UFUNCTION()
	void HandleDeploymentSlotsRotated();

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UUniformGridPanel> GridPanel_BallList;

	UPROPERTY(EditDefaultsOnly, Category = "BallDeck", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPBBallSlotWidget> WBP_BallSlotClass;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDeckSubsystem> DeckSubsystem;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBallSlotWidget>> DeploymentSlotWidgets;
	
};
