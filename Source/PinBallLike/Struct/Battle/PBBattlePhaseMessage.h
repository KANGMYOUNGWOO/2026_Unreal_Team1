// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBBattlePhaseMessage.generated.h"

UENUM(BlueprintType)
enum class EPBBattlePreparationType : uint8
{
	None UMETA(DisplayName = "None"),
	Bumper UMETA(DisplayName = "Bumper"),
	Ball UMETA(DisplayName = "Ball"),
	Boss UMETA(DisplayName = "Boss")
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBattlePreparationCompletedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Message")
	EPBBattlePreparationType PreparationType = EPBBattlePreparationType::None;

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Message")
	bool bSuccess = false;
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBattleBossDeadMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Message")
	TObjectPtr<AActor> BossActor = nullptr;
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBattlePartyDeploymentStartedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Message")
	int32 RemainingLaunchCount = 0;
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBattlePartyLaunchRequestedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Message")
	TObjectPtr<AActor> Requester = nullptr;
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBattlePartyLaunchApprovedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Message")
	int32 RemainingLaunchCountBeforeLaunch = 0;
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBattlePartyLaunchedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Message")
	TObjectPtr<AActor> PartyActor = nullptr;
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBattlePartyAllBallsDeadMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Message")
	TObjectPtr<AActor> PartyActor = nullptr;
};

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBattlePartyShiftRequestedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Message")
	TObjectPtr<AActor> Requester = nullptr;
};
