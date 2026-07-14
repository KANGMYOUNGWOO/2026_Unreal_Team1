// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Components/ActorComponent.h"
#include "PBPartyBattleMessageComponent.generated.h"

class AActor;
struct FPBBattlePhaseChangedMessage;
struct FPBBattlePartyLaunchApprovedMessage;
struct FPBBattleSkillUseRequestedMessage;

struct FPBPartyBattleMessageDependencies
{
	TFunction<void()> PrepareForDeployment;
	TFunction<bool()> LaunchPartyFromReadyPosition;
	TFunction<void(int32)> RequestUseSkill;
	TFunction<AActor*()> GetPartyActor;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBPartyBattleMessageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBPartyBattleMessageComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeDependencies(FPBPartyBattleMessageDependencies InDependencies);
	void BroadcastPartyLaunched(AActor* PartyActor) const;
	void BroadcastPartyAllBallsDead(AActor* PartyActor) const;

private:
	void RegisterMessageListeners();
	void UnregisterMessageListeners();
	void HandleBattlePhaseChangedMessage(FGameplayTag Channel, const FPBBattlePhaseChangedMessage& Message);
	void HandlePartyLaunchApprovedMessage(FGameplayTag Channel, const FPBBattlePartyLaunchApprovedMessage& Message);
	void HandleSkillUseRequestedMessage(FGameplayTag Channel, const FPBBattleSkillUseRequestedMessage& Message);

	FPBPartyBattleMessageDependencies Dependencies;
	FGameplayMessageListenerHandle BattlePhaseChangedListenerHandle;
	FGameplayMessageListenerHandle PartyLaunchApprovedListenerHandle;
	FGameplayMessageListenerHandle SkillUseRequestedListenerHandle;
};
