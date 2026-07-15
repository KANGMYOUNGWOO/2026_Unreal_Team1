#include "PBBattleHUDViewModel.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTableRow.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTierRow.h"

void UPBBattleHUDViewModel::Initialize(UObject* InWorldContextObject)
{
	WorldContextObject = InWorldContextObject;
}

void UPBBattleHUDViewModel::SetSynergyStates(const TArray<FPBSynergyState>& InSynergyStates)
{
	TArray<FPBSynergyViewData> NewViewData;

	const UPBTableDataSubsystem* TableDataSubsystem = GetTableDataSubsystem();
	if (!TableDataSubsystem)
	{
		UE_MVVM_SET_PROPERTY_VALUE(ActiveSynergyViewData, NewViewData);
		return;
	}

	TArray<TTuple<FPBSynergyViewData, int32>> ActiveSynergies;
	for (const FPBSynergyState& SynergyState : InSynergyStates)
	{
		if (!SynergyState.IsValid())
		{
			continue;
		}

		FPBSynergyTableRow SynergyRow;
		if (!TableDataSubsystem->FindSynergyRow(SynergyState.SynergyId, SynergyRow))
		{
			continue;
		}

		FPBSynergyViewData ViewData;
		ViewData.SynergyId = SynergyState.SynergyId;
		ViewData.SynergyName = SynergyRow.DisplayName.IsEmpty()
			? FText::FromName(SynergyState.SynergyId)
			: SynergyRow.DisplayName;
		ViewData.CurrentCount = SynergyState.CurrentCount;
		ViewData.CurrentCountText = FText::AsNumber(SynergyState.CurrentCount);
		ViewData.CountListText = BuildSynergyCountListText(SynergyState.SynergyId, SynergyState.CurrentCount);

		ActiveSynergies.Add(MakeTuple(ViewData, SynergyRow.SortOrder));
	}

	ActiveSynergies.Sort(
		[](const TTuple<FPBSynergyViewData, int32>& Left, const TTuple<FPBSynergyViewData, int32>& Right)
		{
			if (Left.Get<1>() == Right.Get<1>())
			{
				return Left.Get<0>().SynergyId.LexicalLess(Right.Get<0>().SynergyId);
			}

			return Left.Get<1>() < Right.Get<1>();
		});

	NewViewData.Reserve(ActiveSynergies.Num());
	for (const TTuple<FPBSynergyViewData, int32>& ActiveSynergy : ActiveSynergies)
	{
		NewViewData.Add(ActiveSynergy.Get<0>());
	}

	UE_MVVM_SET_PROPERTY_VALUE(ActiveSynergyViewData, NewViewData);
}

void UPBBattleHUDViewModel::ClearSynergyViewData()
{
	TArray<FPBSynergyViewData> EmptyViewData;
	UE_MVVM_SET_PROPERTY_VALUE(ActiveSynergyViewData, EmptyViewData);
}

const UPBTableDataSubsystem* UPBBattleHUDViewModel::GetTableDataSubsystem() const
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UPBTableDataSubsystem>() : nullptr;
}

FText UPBBattleHUDViewModel::BuildSynergyCountListText(const FName SynergyId, const int32 CurrentCount) const
{
	const UPBTableDataSubsystem* TableDataSubsystem = GetTableDataSubsystem();
	if (!TableDataSubsystem || SynergyId.IsNone())
	{
		return FText::GetEmpty();
	}

	TArray<FPBSynergyTierRow> TierRows;
	if (!TableDataSubsystem->GetSynergyTierRows(SynergyId, TierRows))
	{
		return FText::GetEmpty();
	}

	TArray<FString> TierTexts;
	for (const FPBSynergyTierRow& TierRow : TierRows)
	{
		const TCHAR* StyleTag = CurrentCount >= TierRow.RequiredCount ? TEXT("Active") : TEXT("InActive");
		TierTexts.Add(FString::Printf(TEXT("<%s>%d</>"), StyleTag, TierRow.RequiredCount));
	}

	return FText::FromString(FString::Join(TierTexts, TEXT(" ")));
}
