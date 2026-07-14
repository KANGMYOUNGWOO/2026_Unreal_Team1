// Fill out your copyright notice in the Description page of Project Settings.


#include "PBPartyBattleMessageComponent.h"

#include "GameFramework/Actor.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"

UPBPartyBattleMessageComponent::UPBPartyBattleMessageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBPartyBattleMessageComponent::InitializeDependencies(FPBPartyBattleMessageDependencies InDependencies)
{
	Dependencies = MoveTemp(InDependencies);
}

void UPBPartyBattleMessageComponent::BeginPlay()
{
	Super::BeginPlay();
	RegisterMessageListeners();
}

void UPBPartyBattleMessageComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterMessageListeners();
	Super::EndPlay(EndPlayReason);
}

void UPBPartyBattleMessageComponent::RegisterMessageListeners()
{
	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	BattlePhaseChangedListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattlePhaseChangedMessage>(
		GameplayTags::Event_Battle_Phase_Changed,
		this,
		&UPBPartyBattleMessageComponent::HandleBattlePhaseChangedMessage);

	PartyLaunchApprovedListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattlePartyLaunchApprovedMessage>(
		GameplayTags::Event_Battle_Party_Launch_Approved,
		this,
		&UPBPartyBattleMessageComponent::HandlePartyLaunchApprovedMessage);

	SkillUseRequestedListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattleSkillUseRequestedMessage>(
		GameplayTags::Event_Battle_Skill_Use_Requested,
		this,
		&UPBPartyBattleMessageComponent::HandleSkillUseRequestedMessage);
}

void UPBPartyBattleMessageComponent::UnregisterMessageListeners()
{
	if (BattlePhaseChangedListenerHandle.IsValid())
	{
		BattlePhaseChangedListenerHandle.Unregister();
		BattlePhaseChangedListenerHandle = FGameplayMessageListenerHandle();
	}

	if (PartyLaunchApprovedListenerHandle.IsValid())
	{
		PartyLaunchApprovedListenerHandle.Unregister();
		PartyLaunchApprovedListenerHandle = FGameplayMessageListenerHandle();
	}

	if (SkillUseRequestedListenerHandle.IsValid())
	{
		SkillUseRequestedListenerHandle.Unregister();
		SkillUseRequestedListenerHandle = FGameplayMessageListenerHandle();
	}
}

void UPBPartyBattleMessageComponent::HandleBattlePhaseChangedMessage(
	FGameplayTag Channel,
	const FPBBattlePhaseChangedMessage& Message)
{
	(void)Channel;
	if (Message.NewPhase == EPBBattleLevelPhase::BallDeployment)
	{
		if (Dependencies.PrepareForDeployment)
		{
			Dependencies.PrepareForDeployment();
		}
	}
}

void UPBPartyBattleMessageComponent::HandlePartyLaunchApprovedMessage(
	FGameplayTag Channel,
	const FPBBattlePartyLaunchApprovedMessage& Message)
{
	(void)Channel;
	(void)Message;

	if (!Dependencies.LaunchPartyFromReadyPosition || !Dependencies.LaunchPartyFromReadyPosition())
	{
		return;
	}

	BroadcastPartyLaunched(Dependencies.GetPartyActor ? Dependencies.GetPartyActor() : GetOwner());
}

void UPBPartyBattleMessageComponent::HandleSkillUseRequestedMessage(
	FGameplayTag Channel,
	const FPBBattleSkillUseRequestedMessage& Message)
{
	(void)Channel;

	if (Dependencies.RequestUseSkill)
	{
		Dependencies.RequestUseSkill(Message.SkillInputValue);
	}
}

void UPBPartyBattleMessageComponent::BroadcastPartyLaunched(AActor* PartyActor) const
{
	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattlePartyLaunchedMessage Message;
	Message.PartyActor = PartyActor;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_Party_Launched,
		Message);
}

void UPBPartyBattleMessageComponent::BroadcastPartyAllBallsDead(AActor* PartyActor) const
{
	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattlePartyAllBallsDeadMessage Message;
	Message.PartyActor = PartyActor;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_Party_AllBallsDead,
		Message);
}
