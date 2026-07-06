// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "PBGameDataSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, DisplayName = "PB Game Data")
class PINBALLLIKE_API UPBGameDataSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
#pragma region Bumper
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Bumper|Table")
	TSoftObjectPtr<UDataTable> BumperTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Bumper|Table")
	TSoftObjectPtr<UDataTable> BumperTriggerTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Bumper|Table")
	TSoftObjectPtr<UDataTable> BumperEffectTable;
	
#pragma endregion
};
