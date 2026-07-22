// Fill out your copyright notice in the Description page of Project Settings.


#include "Ball/PBBallTableParser.h"

#include "PBSheetParserUtils.h"
#include "Engine/Texture2D.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"

using namespace PBSheetParserUtils;

UPBBallTableParser::UPBBallTableParser()
{
	DataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Ball");
	DataAssetPreset.NameFormat = TEXT("DA_{0}");
	BallIconPreset.FolderPath.Path = TEXT("/Game/Resources/Ball/Icon");
	BallIconPreset.NameFormat = TEXT("Icon_{0}");
	BallSpritePreset.FolderPath.Path = TEXT("/Game/Resources/Ball/Sprite");
	BallSpritePreset.NameFormat = TEXT("Sprite_{0}");
	SynergyIconPreset.FolderPath.Path = TEXT("/Game/Resources/Synergy");
	SynergyIconPreset.NameFormat = TEXT("Icon_{0}");
	PowerFlipIconPreset.FolderPath.Path = TEXT("/Game/Resources/Ball/PowerFlip");
	PowerFlipIconPreset.NameFormat = TEXT("Icon_{0}");
}

const TCHAR* UPBBallTableParser::GetParserName() const
{
	return TEXT("Ball");
}

UScriptStruct* UPBBallTableParser::GetRowStruct() const
{
	return FPBBallTableRow::StaticStruct();
}

bool UPBBallTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBBallTableRow NewRow;
	NewRow.DisplayName = FText::FromString(RowData.FindRef(TEXT("DisplayName")));
	NewRow.DescriptionKey = FText::FromString(RowData.FindRef(TEXT("DescriptionKey")));
	NewRow.PowerFlipType = ParseEnumValue(RowData.FindRef(TEXT("PowerFlipType")), EPBPowerFlipType::Sword);
	NewRow.RaceTypes = ParseEnumArray<EPBBallRaceType>(RowData.FindRef(TEXT("RaceType")));

	const FString ClassTypeText = TrimCell(RowData.FindRef(TEXT("ClassType")));
	NewRow.ClassType = ParseEnumValue(ClassTypeText, EPBBallClassType::None);
	if (NewRow.ClassType == EPBBallClassType::None
		&& !ClassTypeText.Equals(TEXT("None"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Sheet][Ball] Invalid or empty ClassType. RowName=%s ClassType=%s"),
			*RowName.ToString(),
			*ClassTypeText);
	}

	NewRow.DefaultSkillIds = ParseNameArray(RowData.FindRef(TEXT("DefaultSkillIds")));
	NewRow.VoiceType = ParseEnumValue(RowData.FindRef(TEXT("VoiceType")), EPBBallVoiceType::Girl);
	NewRow.ShopId = FName(*TrimCell(RowData.FindRef(TEXT("ShopId"))));

	(void)SetupBallDataAsset(RowName, NewRow);

	TargetTable->AddRow(RowName, NewRow);
	return true;
}

UPBBallDataAsset* UPBBallTableParser::SetupBallDataAsset(
	const FName RowName,
	const FPBBallTableRow& BallRow) const
{
	UPBBallDataAsset* BallDataAsset =
		GetOrCreateDataAsset<UPBBallDataAsset>(DataAssetPreset, RowName, TEXT("Ball"));
	if (!IsValid(BallDataAsset))
	{
		return nullptr;
	}

	if (BallIconPreset.IsValid())
	{
		BallDataAsset->BallIcon = FindObject<UTexture2D>(BallIconPreset, RowName);
	}

	if (BallSpritePreset.IsValid())
	{
		BallDataAsset->BallSprite = FindObject<UTexture2D>(BallSpritePreset, RowName);
	}

	const UEnum* PowerFlipEnum = StaticEnum<EPBPowerFlipType>();
	if (PowerFlipIconPreset.IsValid() && PowerFlipEnum)
	{
		const FName PowerFlipIconId(*PowerFlipEnum->GetNameStringByValue(static_cast<int64>(BallRow.PowerFlipType)));
		BallDataAsset->PowerFlipIcon = FindObject<UTexture2D>(PowerFlipIconPreset, PowerFlipIconId);
	}

	BallDataAsset->RaceIcons.Reset();
	const UEnum* RaceEnum = StaticEnum<EPBBallRaceType>();
	if (SynergyIconPreset.IsValid() && RaceEnum)
	{
		for (const EPBBallRaceType RaceType : BallRow.RaceTypes)
		{
			const FName RaceIconId(*RaceEnum->GetNameStringByValue(static_cast<int64>(RaceType)));
			BallDataAsset->RaceIcons.Add(FindObject<UTexture2D>(SynergyIconPreset, RaceIconId));
		}
	}

	const UEnum* ClassEnum = StaticEnum<EPBBallClassType>();
	if (SynergyIconPreset.IsValid()
		&& ClassEnum
		&& BallRow.ClassType != EPBBallClassType::None)
	{
		const FName ClassIconId(*ClassEnum->GetNameStringByValue(static_cast<int64>(BallRow.ClassType)));
		BallDataAsset->ClassIcon = FindObject<UTexture2D>(SynergyIconPreset, ClassIconId);
	}
	else
	{
		BallDataAsset->ClassIcon = nullptr;
	}

	(void)BallDataAsset->MarkPackageDirty();
	return BallDataAsset;
}
