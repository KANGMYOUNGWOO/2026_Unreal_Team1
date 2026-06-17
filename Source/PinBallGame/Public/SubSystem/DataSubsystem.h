// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PINBALLGAME_API UDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public :
	void InitializeData();

private:
	

};
