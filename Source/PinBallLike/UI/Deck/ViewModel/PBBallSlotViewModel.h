// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PBBallSlotViewModel.generated.h"

/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBBallSlotViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
private:
	int32 SlotIndex;
	
};
