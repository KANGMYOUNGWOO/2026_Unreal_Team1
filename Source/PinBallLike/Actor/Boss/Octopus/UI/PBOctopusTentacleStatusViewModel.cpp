#include "PBOctopusTentacleStatusViewModel.h"

#include "PinBallLike/Actor/Boss/Octopus/PBOctopusBoss.h"
#include "PinBallLike/Actor/Boss/Octopus/PBOctopusTentacle.h"

void UPBOctopusTentacleStatusViewModel::SetOctopusBoss(APBOctopusBoss* NewOctopusBoss)
{
	if (OctopusBoss == NewOctopusBoss)
	{
		BindTentacleEvents();
		RefreshTentacleStatus();
		return;
	}

	ClearOctopusBoss();
	OctopusBoss = NewOctopusBoss;
	if (OctopusBoss)
	{
		OctopusBoss->OnTentaclesChanged.AddUniqueDynamic(
			this,
			&UPBOctopusTentacleStatusViewModel::HandleTentaclesChanged);
	}

	BindTentacleEvents();
	RefreshTentacleStatus();
}

void UPBOctopusTentacleStatusViewModel::ClearOctopusBoss()
{
	UnbindTentacleEvents();
	if (OctopusBoss)
	{
		OctopusBoss->OnTentaclesChanged.RemoveDynamic(
			this,
			&UPBOctopusTentacleStatusViewModel::HandleTentaclesChanged);
	}

	OctopusBoss = nullptr;
	HandleTentacle1HPChanged(0, 1);
	HandleTentacle2HPChanged(0, 1);
	HandleTentacle3HPChanged(0, 1);
	HandleTentacle4HPChanged(0, 1);
}

void UPBOctopusTentacleStatusViewModel::BeginDestroy()
{
	ClearOctopusBoss();
	Super::BeginDestroy();
}

void UPBOctopusTentacleStatusViewModel::HandleTentaclesChanged()
{
	BindTentacleEvents();
	RefreshTentacleStatus();
}

void UPBOctopusTentacleStatusViewModel::HandleTentacle1HPChanged(
	const int32 CurrentTentacleHP,
	const int32 MaxTentacleHP)
{
	UE_MVVM_SET_PROPERTY_VALUE(Tentacle1HPPercent, CalculateHPPercent(CurrentTentacleHP, MaxTentacleHP));
}

void UPBOctopusTentacleStatusViewModel::HandleTentacle2HPChanged(
	const int32 CurrentTentacleHP,
	const int32 MaxTentacleHP)
{
	UE_MVVM_SET_PROPERTY_VALUE(Tentacle2HPPercent, CalculateHPPercent(CurrentTentacleHP, MaxTentacleHP));
}

void UPBOctopusTentacleStatusViewModel::HandleTentacle3HPChanged(
	const int32 CurrentTentacleHP,
	const int32 MaxTentacleHP)
{
	UE_MVVM_SET_PROPERTY_VALUE(Tentacle3HPPercent, CalculateHPPercent(CurrentTentacleHP, MaxTentacleHP));
}

void UPBOctopusTentacleStatusViewModel::HandleTentacle4HPChanged(
	const int32 CurrentTentacleHP,
	const int32 MaxTentacleHP)
{
	UE_MVVM_SET_PROPERTY_VALUE(Tentacle4HPPercent, CalculateHPPercent(CurrentTentacleHP, MaxTentacleHP));
}

void UPBOctopusTentacleStatusViewModel::BindTentacleEvents()
{
	UnbindTentacleEvents();
	if (!OctopusBoss)
	{
		return;
	}

	Tentacles.SetNum(4);
	for (int32 TentacleIndex = 0; TentacleIndex < Tentacles.Num(); ++TentacleIndex)
	{
		Tentacles[TentacleIndex] = OctopusBoss->GetTentacle(TentacleIndex);
	}

	if (Tentacles[0])
	{
		Tentacles[0]->OnTentacleHPChanged.AddUniqueDynamic(
			this,
			&UPBOctopusTentacleStatusViewModel::HandleTentacle1HPChanged);
	}
	if (Tentacles[1])
	{
		Tentacles[1]->OnTentacleHPChanged.AddUniqueDynamic(
			this,
			&UPBOctopusTentacleStatusViewModel::HandleTentacle2HPChanged);
	}
	if (Tentacles[2])
	{
		Tentacles[2]->OnTentacleHPChanged.AddUniqueDynamic(
			this,
			&UPBOctopusTentacleStatusViewModel::HandleTentacle3HPChanged);
	}
	if (Tentacles[3])
	{
		Tentacles[3]->OnTentacleHPChanged.AddUniqueDynamic(
			this,
			&UPBOctopusTentacleStatusViewModel::HandleTentacle4HPChanged);
	}
}

void UPBOctopusTentacleStatusViewModel::UnbindTentacleEvents()
{
	if (Tentacles.IsValidIndex(0) && Tentacles[0])
	{
		Tentacles[0]->OnTentacleHPChanged.RemoveDynamic(
			this,
			&UPBOctopusTentacleStatusViewModel::HandleTentacle1HPChanged);
	}
	if (Tentacles.IsValidIndex(1) && Tentacles[1])
	{
		Tentacles[1]->OnTentacleHPChanged.RemoveDynamic(
			this,
			&UPBOctopusTentacleStatusViewModel::HandleTentacle2HPChanged);
	}
	if (Tentacles.IsValidIndex(2) && Tentacles[2])
	{
		Tentacles[2]->OnTentacleHPChanged.RemoveDynamic(
			this,
			&UPBOctopusTentacleStatusViewModel::HandleTentacle3HPChanged);
	}
	if (Tentacles.IsValidIndex(3) && Tentacles[3])
	{
		Tentacles[3]->OnTentacleHPChanged.RemoveDynamic(
			this,
			&UPBOctopusTentacleStatusViewModel::HandleTentacle4HPChanged);
	}

	Tentacles.Reset();
}

void UPBOctopusTentacleStatusViewModel::RefreshTentacleStatus()
{
	HandleTentacle1HPChanged(
		Tentacles.IsValidIndex(0) && Tentacles[0] ? Tentacles[0]->GetCurrentTentacleHP() : 0,
		Tentacles.IsValidIndex(0) && Tentacles[0] ? Tentacles[0]->GetMaxTentacleHP() : 1);
	HandleTentacle2HPChanged(
		Tentacles.IsValidIndex(1) && Tentacles[1] ? Tentacles[1]->GetCurrentTentacleHP() : 0,
		Tentacles.IsValidIndex(1) && Tentacles[1] ? Tentacles[1]->GetMaxTentacleHP() : 1);
	HandleTentacle3HPChanged(
		Tentacles.IsValidIndex(2) && Tentacles[2] ? Tentacles[2]->GetCurrentTentacleHP() : 0,
		Tentacles.IsValidIndex(2) && Tentacles[2] ? Tentacles[2]->GetMaxTentacleHP() : 1);
	HandleTentacle4HPChanged(
		Tentacles.IsValidIndex(3) && Tentacles[3] ? Tentacles[3]->GetCurrentTentacleHP() : 0,
		Tentacles.IsValidIndex(3) && Tentacles[3] ? Tentacles[3]->GetMaxTentacleHP() : 1);
}

float UPBOctopusTentacleStatusViewModel::CalculateHPPercent(const int32 CurrentHP, const int32 MaxHP)
{
	if (MaxHP <= 0)
	{
		return 0.0f;
	}

	return FMath::Clamp(static_cast<float>(CurrentHP) / static_cast<float>(MaxHP), 0.0f, 1.0f);
}
