// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBSellZoneWidget.generated.h"

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
	void BindBallItemDragEvents();
	void UnbindBallItemDragEvents();
	void HandleBallItemDragStarted(int32 BallInstanceId);
	void EnsureSellZoneViewModel();
	bool ApplyViewModelToWidget();
	
	UPROPERTY(Transient)
	TObjectPtr<UPBSellZoneViewModel> SellZoneViewModel;

	bool bSellEnabled = false;
};
