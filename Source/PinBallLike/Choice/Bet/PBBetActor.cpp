// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBetActor.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include  "PinBallLike/Choice/UI/PBBettingWidget.h"
#include  "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Table/Ball/PBBallAssetIds.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"

namespace
{
	struct FBetNationCandidate
	{
		FName BallId = NAME_None;
		FText DisplayName;
		UTexture2D* BallSprite = nullptr;
	};

	bool ContainsKoreanCharacter(const FText& Text)
	{
		const FString String = Text.ToString();
		for (const TCHAR Character : String)
		{
			if ((Character >= 0xAC00 && Character <= 0xD7A3)
				|| (Character >= 0x3131 && Character <= 0x318E))
			{
				return true;
			}
		}

		return false;
	}
}

void APBBetActor::OpenAbility()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;	
	}
	
	if (!BetWidget)
	{
		if (!BetWidgetClass)
		{
			return;
		}

		BetWidget = CreateWidget<UPBBettingWidget>(PC,BetWidgetClass);
		if (!BetWidget)
		{
			return;
		}
		
		BetWidget->AddToViewport();
	}
	
	PC->bShowMouseCursor = true;
	PC->SetInputMode(FInputModeGameAndUI());
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;
	
	BindWidget(BetWidget);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPBPlayerDataSubsystem* PlayerDataSubsystem =
			GameInstance->GetSubsystem<UPBPlayerDataSubsystem>())
		{
			BetWidget->SetAvailableGold(
				PlayerDataSubsystem->GetCurrentGold());
		}
	}

	SetupBetNationData();

	BetWidget->PlayIntroAnimation();
}

void APBBetActor::HandleExitStart(FGameplayTag Exit, const FPBChoiceType& Message)
{
	
	if (Message.Exit == 1)
	{
		if (BetWidget)
		{
			BetWidget->RemoveFromParent();
			BetWidget = nullptr;
		}
		
		APlayerController* PC = UGameplayStatics::GetPlayerController(this,0);
		if (PC)
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}
	}
}

void APBBetActor::BindWidget(UPBBettingWidget* InWidget)
{
	if (!IsValid(InWidget))
	{
		return;
	}

	BettingWidget = InWidget;

	BettingWidget->OnBetSelected.AddUniqueDynamic(
		this,
		&APBBetActor::HandleBetSelected);

	BettingWidget->OnBetResultAnimationsFinished.AddUniqueDynamic(
		this,
		&APBBetActor::HandleBetResultAnimationsFinished);

	BettingWidget->OnBetExitRequested.AddUniqueDynamic(
		this,
		&APBBetActor::HandleExitRequested);
}

void APBBetActor::HandleBetSelected(int32 SelectedIndex, int32 BetGold)
{
	const FPBBettingResult Result =
		ResolveBet(SelectedIndex);
	ApplyBetGoldResult(Result.bWin, BetGold);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"Bet Result | Selected: %d | Winner: %d | Win: %s"),
		Result.SelectedIndex,
		Result.WinnerIndex,
		Result.bWin ? TEXT("True") : TEXT("False"));

	if (!IsValid(BettingWidget))
	{
		FinishBet();
		return;
	}

	BettingWidget->PlayBetResultAnimations(Result);
}

FPBBettingResult APBBetActor::ResolveBet(int32 SelectedIndex)
{
	FPBBettingResult Result;

	Result.SelectedIndex = SelectedIndex;

	// 현재는 단순 50 : 50
	Result.WinnerIndex = FMath::RandRange(0, 1);

	Result.bWin =
		Result.SelectedIndex == Result.WinnerIndex;

	return Result;
}

void APBBetActor::ApplyBetGoldResult(bool IsWin, int32 BetGold)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UPBPlayerDataSubsystem* PlayerDataSubsystem =
		GameInstance->GetSubsystem<UPBPlayerDataSubsystem>();
	if (!PlayerDataSubsystem)
	{
		return;
	}

	const int32 EarnedGold = IsWin ? BetGold : -BetGold;
	const int32 CurrentGold = PlayerDataSubsystem->GetCurrentGold();

	PlayerDataSubsystem->GainGold(EarnedGold);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Bet] CurrentGold=%d EarnedGold=%d GoldAfter=%d"),
		CurrentGold,
		EarnedGold,
		PlayerDataSubsystem->GetCurrentGold());
}

void APBBetActor::SetupBetNationData()
{
	if (!BetWidget)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UPBTableDataSubsystem* TableDataSubsystem = GameInstance
		? GameInstance->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	if (!TableDataSubsystem)
	{
		return;
	}

	TArray<FName> BallIds;
	TArray<FPBBallTableRow> BallRows;
	if (!TableDataSubsystem->GetAllBallRows(BallIds, BallRows))
	{
		return;
	}

	TArray<FBetNationCandidate> Candidates;
	const int32 BallCount = FMath::Min(BallIds.Num(), BallRows.Num());
	UAssetManager& AssetManager = UAssetManager::Get();
	for (int32 BallIndex = 0; BallIndex < BallCount; ++BallIndex)
	{
		const FPBBallTableRow& BallRow = BallRows[BallIndex];
		if (BallRow.DisplayName.IsEmpty()
			|| !ContainsKoreanCharacter(BallRow.DisplayName))
		{
			continue;
		}

		const FPrimaryAssetId BallAssetId(
			PBBallAssetIds::Type::BallData,
			BallIds[BallIndex]);
		const FSoftObjectPath BallDataAssetPath =
			AssetManager.GetPrimaryAssetPath(BallAssetId);
		const UPBBallDataAsset* BallDataAsset =
			Cast<UPBBallDataAsset>(BallDataAssetPath.TryLoad());
		UTexture2D* BallSprite = BallDataAsset
			? BallDataAsset->BallSprite.LoadSynchronous()
			: nullptr;
		if (!BallSprite)
		{
			continue;
		}

		FBetNationCandidate& Candidate = Candidates.AddDefaulted_GetRef();
		Candidate.BallId = BallIds[BallIndex];
		Candidate.DisplayName = BallRow.DisplayName;
		Candidate.BallSprite = BallSprite;
	}

	if (Candidates.Num() < 2)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[Bet] At least two balls with Korean DisplayName and BallSprite are required. Count=%d"),
			Candidates.Num());
		return;
	}

	const int32 FirstCandidateIndex =
		FMath::RandRange(0, Candidates.Num() - 1);
	const FBetNationCandidate FirstCandidate =
		Candidates[FirstCandidateIndex];
	Candidates.RemoveAtSwap(FirstCandidateIndex);

	const int32 SecondCandidateIndex =
		FMath::RandRange(0, Candidates.Num() - 1);
	const FBetNationCandidate& SecondCandidate =
		Candidates[SecondCandidateIndex];

	BetWidget->SetNationData(
		FirstCandidate.DisplayName,
		FirstCandidate.BallSprite,
		SecondCandidate.DisplayName,
		SecondCandidate.BallSprite);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Bet] Nation candidates selected. Left=%s Right=%s"),
		*FirstCandidate.BallId.ToString(),
		*SecondCandidate.BallId.ToString());
}

void APBBetActor::FinishBet()
{
	FPBChoiceType Message;
	Message.Exit = 1;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_UI_Choice_Exit,
		Message);
}

void APBBetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Sets default values
APBBetActor::APBBetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void APBBetActor::HandleBetResultAnimationsFinished()
{
	FinishBet();
}

void APBBetActor::HandleExitRequested()
{
	FinishBet();
}

// Called when the game starts or when spawned
void APBBetActor::BeginPlay()
{
	Super::BeginPlay();
	
	UGameplayMessageSubsystem& MessageSubsystem =
	UGameplayMessageSubsystem::Get(this);
	
	ExitStartHandle =
		MessageSubsystem.RegisterListener<FPBChoiceType>(
			GameplayTags::Event_UI_Choice_Exit,
			this,
			&APBBetActor::HandleExitStart);
}

// Called every frame
