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
#pragma region Collection

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Collection|Table")
	TSoftObjectPtr<UDataTable> CollectionTable;

#pragma endregion
#pragma region Bumper
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Bumper|Table")
	TSoftObjectPtr<UDataTable> BumperTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Bumper|Table")
	TSoftObjectPtr<UDataTable> BumperTriggerTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Bumper|Table")
	TSoftObjectPtr<UDataTable> BumperEffectTable;
	
#pragma endregion
	
#pragma region Ball

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Ball|Table")
	TSoftObjectPtr<UDataTable> BallTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Ball|Table")
	TSoftObjectPtr<UDataTable> BallStarLevelTable;
	
	
#pragma endregion

#pragma region Shop

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Shop|Table")
	TSoftObjectPtr<UDataTable> ShopTable;
	
#pragma endregion

#pragma  region Choice
	UPROPERTY(EditDefaultsOnly, Config, Category="Choice|GameData")
	TSoftObjectPtr<UDataTable> ChoiceTable;
	
#pragma endregion 
#pragma region Boss

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Boss|Table")
	TSoftObjectPtr<UDataTable> Boss;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Boss|Table")
	TSoftObjectPtr<UDataTable> BossHitPoint;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Boss|Table")
	TSoftObjectPtr<UDataTable> BossPattern;
	
#pragma endregion 

#pragma region StatusEffect

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "StatusEffect|Table")
	TSoftObjectPtr<UDataTable> StatusEffectTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "StatusEffect|Table")
	TSoftObjectPtr<UDataTable> StatusEffectModifierTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "StatusEffect|Table")
	TSoftObjectPtr<UDataTable> StatusEffectTriggerTable;

#pragma endregion
};
