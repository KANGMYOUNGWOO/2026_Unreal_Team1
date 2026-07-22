// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallComboComponent.h"
#include "PBBallEffectRuntimeComponent.h"
#include "PinBallLike/GameState/PBBattleGameState.h"


UPBBallComboComponent::UPBBallComboComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBallComboComponent::BeginPlay()
{
	Super::BeginPlay();

	if (APBBattleGameState* BattleGameState = GetBattleGameState())
	{
		BattleGameState->OnBattleComboChanged.AddUniqueDynamic(this, &UPBBallComboComponent::HandleBattleComboChanged);
		UE_LOG(LogTemp, Log, TEXT("[Combo] BallComboComponent bound to BattleGameState. Owner=%s CurrentCombo=%d"), *GetNameSafe(GetOwner()), BattleGameState->GetCombo());
		HandleBattleComboChanged(BattleGameState->GetCombo());
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Combo] BallComboComponent failed to bind BattleGameState. Owner=%s World=%s"), *GetNameSafe(GetOwner()), *GetNameSafe(GetWorld()));
}

void UPBBallComboComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APBBattleGameState* BattleGameState = GetBattleGameState())
	{
		BattleGameState->OnBattleComboChanged.RemoveDynamic(this, &UPBBallComboComponent::HandleBattleComboChanged);
	}

	Super::EndPlay(EndPlayReason);
}

int32 UPBBallComboComponent::GetCombo() const
{
	const APBBattleGameState* BattleGameState = GetBattleGameState();
	return BattleGameState ? BattleGameState->GetCombo() : 0;
}

void UPBBallComboComponent::SetCombo(int32 Value)
{
	if (APBBattleGameState* BattleGameState = GetBattleGameState())
	{
		UE_LOG(LogTemp, Log, TEXT("[Combo] SetCombo forwarded to BattleGameState. Owner=%s Value=%d Previous=%d"), *GetNameSafe(GetOwner()), Value, BattleGameState->GetCombo());
		BattleGameState->SetCombo(Value);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Combo] SetCombo failed because BattleGameState is invalid. Owner=%s Value=%d"), *GetNameSafe(GetOwner()), Value);
}

void UPBBallComboComponent::AddCombo(int32 Delta)
{
	if (Delta == 0)
	{
		return;
	}

	if (APBBattleGameState* BattleGameState = GetBattleGameState())
	{
		UE_LOG(LogTemp, Log, TEXT("[Combo] AddCombo forwarded to BattleGameState. Owner=%s Delta=%d Previous=%d"), *GetNameSafe(GetOwner()), Delta, BattleGameState->GetCombo());
		BattleGameState->AddCombo(Delta);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Combo] AddCombo failed because BattleGameState is invalid. Owner=%s Delta=%d"), *GetNameSafe(GetOwner()), Delta);
}

bool UPBBallComboComponent::TryConsumeCombo(int32 Cost)
{
	if (APBBattleGameState* BattleGameState = GetBattleGameState())
	{
		const bool bConsumed = BattleGameState->TryConsumeCombo(Cost);
		UE_LOG(LogTemp, Log, TEXT("[Combo] TryConsumeCombo forwarded to BattleGameState. Owner=%s Cost=%d Result=%s Current=%d"), *GetNameSafe(GetOwner()), Cost, bConsumed ? TEXT("true") : TEXT("false"), BattleGameState->GetCombo());
		return bConsumed;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Combo] TryConsumeCombo failed because BattleGameState is invalid. Owner=%s Cost=%d"), *GetNameSafe(GetOwner()), Cost);
	return false;
}

void UPBBallComboComponent::ResetCombo()
{
	if (APBBattleGameState* BattleGameState = GetBattleGameState())
	{
		UE_LOG(LogTemp, Log, TEXT("[Combo] ResetCombo forwarded to BattleGameState. Owner=%s Previous=%d"), *GetNameSafe(GetOwner()), BattleGameState->GetCombo());
		BattleGameState->ResetCombo();
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Combo] ResetCombo failed because BattleGameState is invalid. Owner=%s"), *GetNameSafe(GetOwner()));
}

APBBattleGameState* UPBBallComboComponent::GetBattleGameState() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetGameState<APBBattleGameState>() : nullptr;
}

void UPBBallComboComponent::HandleBattleComboChanged(const int32 CurrentCombo)
{
	UE_LOG(LogTemp, Log, TEXT("[Combo] BallComboComponent received BattleGameState combo change. Owner=%s Current=%d"), *GetNameSafe(GetOwner()), CurrentCombo);
	OnComboChanged.Broadcast(CurrentCombo);

	if (UPBBallEffectRuntimeComponent* EffectRuntimeComponent =
		GetOwner() ? GetOwner()->FindComponentByClass<UPBBallEffectRuntimeComponent>() : nullptr)
	{
		EffectRuntimeComponent->HandleComboChanged(CurrentCombo);
	}
}
