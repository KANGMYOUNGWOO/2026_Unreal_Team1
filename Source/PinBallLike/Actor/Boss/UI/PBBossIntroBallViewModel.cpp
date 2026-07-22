#include "PBBossIntroBallViewModel.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "PinBallLike/Struct/Deck/PBDeckOwnedBallData.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckAssetLoadService.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"

void UPBBossIntroBallViewModel::RefreshBallSprites(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	const UPBBallDeckSubsystem* BallDeckSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;

	if (!BallDeckSubsystem)
	{
		ClearBallSprites();
		return;
	}

	FSlateBrush NewBall01Brush;
	FSlateBrush NewBall02Brush;
	FSlateBrush NewBall03Brush;
	FText NewBall01NameText;
	FText NewBall02NameText;
	FText NewBall03NameText;
	ESlateVisibility NewBall01Visibility = ESlateVisibility::Collapsed;
	ESlateVisibility NewBall02Visibility = ESlateVisibility::Collapsed;
	ESlateVisibility NewBall03Visibility = ESlateVisibility::Collapsed;

	ResolveBallViewData(BallDeckSubsystem, 0, NewBall01Brush, NewBall01NameText, NewBall01Visibility);
	ResolveBallViewData(BallDeckSubsystem, 1, NewBall02Brush, NewBall02NameText, NewBall02Visibility);
	ResolveBallViewData(BallDeckSubsystem, 2, NewBall03Brush, NewBall03NameText, NewBall03Visibility);

	UE_MVVM_SET_PROPERTY_VALUE(Ball01Brush, NewBall01Brush);
	UE_MVVM_SET_PROPERTY_VALUE(Ball02Brush, NewBall02Brush);
	UE_MVVM_SET_PROPERTY_VALUE(Ball03Brush, NewBall03Brush);
	UE_MVVM_SET_PROPERTY_VALUE(Ball01NameText, NewBall01NameText);
	UE_MVVM_SET_PROPERTY_VALUE(Ball02NameText, NewBall02NameText);
	UE_MVVM_SET_PROPERTY_VALUE(Ball03NameText, NewBall03NameText);
	UE_MVVM_SET_PROPERTY_VALUE(Ball01Visibility, NewBall01Visibility);
	UE_MVVM_SET_PROPERTY_VALUE(Ball02Visibility, NewBall02Visibility);
	UE_MVVM_SET_PROPERTY_VALUE(Ball03Visibility, NewBall03Visibility);
}

void UPBBossIntroBallViewModel::ClearBallSprites()
{
	UE_MVVM_SET_PROPERTY_VALUE(Ball01Brush, FSlateBrush());
	UE_MVVM_SET_PROPERTY_VALUE(Ball02Brush, FSlateBrush());
	UE_MVVM_SET_PROPERTY_VALUE(Ball03Brush, FSlateBrush());
	UE_MVVM_SET_PROPERTY_VALUE(Ball01NameText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(Ball02NameText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(Ball03NameText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(Ball01Visibility, ESlateVisibility::Collapsed);
	UE_MVVM_SET_PROPERTY_VALUE(Ball02Visibility, ESlateVisibility::Collapsed);
	UE_MVVM_SET_PROPERTY_VALUE(Ball03Visibility, ESlateVisibility::Collapsed);
}

void UPBBossIntroBallViewModel::ResolveBallViewData(
	const UPBBallDeckSubsystem* BallDeckSubsystem,
	const int32 SlotIndex,
	FSlateBrush& OutBallBrush,
	FText& OutBallNameText,
	ESlateVisibility& OutBallVisibility) const
{
	OutBallBrush = FSlateBrush();
	OutBallNameText = FText::GetEmpty();
	OutBallVisibility = ESlateVisibility::Collapsed;

	if (!BallDeckSubsystem)
	{
		return;
	}

	const int32 BallInstanceId = BallDeckSubsystem->GetSlotBallInstanceId(
		EPBBallDeckSlotType::Deployment,
		SlotIndex);
	const FPBDeckOwnedBallData* OwnedBallData = BallDeckSubsystem->GetOwnedBallData(BallInstanceId);
	const UPBBallDeckAssetLoadService* AssetLoadService = BallDeckSubsystem->GetAssetLoadService();
	const UPBBallDataAsset* BallDataAsset = AssetLoadService
		? AssetLoadService->GetLoadedBallDataAsset(BallInstanceId)
		: nullptr;
	UTexture2D* BallSprite = BallDataAsset ? BallDataAsset->BallSprite.Get() : nullptr;
	if (!OwnedBallData || !BallSprite)
	{
		return;
	}

	const UGameInstance* GameInstance = BallDeckSubsystem->GetGameInstance();
	const UPBTableDataSubsystem* TableDataSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBTableDataSubsystem>() : nullptr;
	FPBBallTableRow BallTableRow;
	if (TableDataSubsystem && TableDataSubsystem->FindBallRow(OwnedBallData->BallId, BallTableRow))
	{
		OutBallNameText = BallTableRow.DisplayName;
	}

	OutBallBrush.SetResourceObject(BallSprite);
	OutBallVisibility = ESlateVisibility::HitTestInvisible;
}
