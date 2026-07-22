#include "PBShellGameActor.h"

#include "PBShellCupActor.h"
#include "AssetRegistry/AssetData.h"
#include "Components/SceneComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Struct/Choice/PBChoiceType.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Subsystem/PBSoundSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Subsystem/PBUIManagerSubsystem.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Table/Ball/PBBallAssetIds.h"
#include "PinBallLike/UI/PBUserWidget.h"

APBShellGameActor::APBShellGameActor()
{
    PrimaryActorTick.bCanEverTick = true;

    GoldRewardIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(
        TEXT("/Game/Resources/UI/T_UI_Gold.T_UI_Gold")));

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    LeftSlot = CreateDefaultSubobject<USceneComponent>(TEXT("LeftSlot"));
    LeftSlot->SetupAttachment(Root);
    LeftSlot->SetRelativeLocation(FVector(0.f, -250.f, 0.f));

    CenterSlot = CreateDefaultSubobject<USceneComponent>(TEXT("CenterSlot"));
    CenterSlot->SetupAttachment(Root);
    CenterSlot->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

    RightSlot = CreateDefaultSubobject<USceneComponent>(TEXT("RightSlot"));
    RightSlot->SetupAttachment(Root);
    RightSlot->SetRelativeLocation(FVector(0.f, 250.f, 0.f));
}

void APBShellGameActor::BeginPlay()
{
    Super::BeginPlay();

    CupSlots =
    {
        LeftSlot,
        CenterSlot,
        RightSlot
    };

    if (!InitializeCups())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[ShellGame] Cup initialization failed."));

        SetActorTickEnabled(false);
        return;
    }

    ResetShellGame();

    if (bAutoStartOnBeginPlay)
    {
        StartShellGame();
    }
}

void APBShellGameActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState == EPBShellGameState::Shuffling)
    {
        UpdateCurrentShuffle(DeltaTime);
    }
}

bool APBShellGameActor::InitializeCups()
{
    if (Cups.Num() != 3)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[ShellGame] Exactly 3 cups are required. Current=%d"),
            Cups.Num());

        return false;
    }

    if (CupSlots.Num() != 3)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[ShellGame] Exactly 3 cup slots are required. Current=%d"),
            CupSlots.Num());

        return false;
    }

    for (int32 CupIndex = 0; CupIndex < Cups.Num(); ++CupIndex)
    {
        APBShellCupActor* Cup = Cups[CupIndex];
        USceneComponent* Slot = CupSlots[CupIndex];

        if (!IsValid(Cup) || !IsValid(Slot))
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT("[ShellGame] Invalid cup or slot. Index=%d"),
                CupIndex);

            return false;
        }

        Cup->SetCupIndex(CupIndex);
        Cup->SetOwnerGame(this);
        Cup->OnRaiseCupFinished.AddUObject(
            this,
            &APBShellGameActor::HandleCupRaiseFinished);
        Cup->OnLowerCupFinished.AddUObject(
            this,
            &APBShellGameActor::HandleCupLowerFinished);

        Cup->SetActorLocationAndRotation(
            Slot->GetComponentLocation(),
            Slot->GetComponentRotation());
    }

    return true;
}

void APBShellGameActor::ResetShellGame()
{
    CurrentState = EPBShellGameState::Idle;

    WinningCupIndex = INDEX_NONE;

    CurrentShuffleCommandIndex = 0;
    FirstMovingCupIndex = INDEX_NONE;
    SecondMovingCupIndex = INDEX_NONE;

    CurrentShuffleElapsed = 0.f;
    CurrentShuffleDuration = 0.f;
    PendingRevealCupAnimationCount = 0;
    bRewardGranted = false;
    CurrentReward = FPBShellGameReward();
    CurrentRewardIcon = nullptr;

    GetWorldTimerManager().ClearTimer(ShowBallTimerHandle);
    GetWorldTimerManager().ClearTimer(RevealTimerHandle);

    ShuffleCommands.Reset();

    CupSlotIndices = { 0, 1, 2 };

    UpdateCupSlotLocations();

    for (int32 CupIndex = 0; CupIndex < Cups.Num(); ++CupIndex)
    {
        if (!Cups.IsValidIndex(CupIndex) ||
            !CupSlots.IsValidIndex(CupIndex) ||
            !IsValid(Cups[CupIndex]) ||
            !IsValid(CupSlots[CupIndex]))
        {
            continue;
        }

        Cups[CupIndex]->SetActorLocationAndRotation(
            CupSlots[CupIndex]->GetComponentLocation(),
            CupSlots[CupIndex]->GetComponentRotation());

        Cups[CupIndex]->ResetCupPresentation();
        Cups[CupIndex]->SetSelectionEnabled(false);
        Cups[CupIndex]->HidePrize();
        Cups[CupIndex]->SetPrizeSprite(nullptr);
        Cups[CupIndex]->SetPrizeScale(1.f);
    }
}

bool APBShellGameActor::PrepareReward()
{
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        return false;
    }

    const UPBBallDeckSubsystem* DeckSubsystem =
        GameInstance->GetSubsystem<UPBBallDeckSubsystem>();
    const UPBTableDataSubsystem* TableDataSubsystem =
        GameInstance->GetSubsystem<UPBTableDataSubsystem>();

    if (!DeckSubsystem || !TableDataSubsystem)
    {
        return false;
    }

    if (!DeckSubsystem->HasEmptyDeckSlot())
    {
        PrepareGoldReward();
        return CurrentReward.IsValid();
    }

    TArray<FName> CandidateBallIds;
    TArray<FPBBallTableRow> BallRows;
    if (!TableDataSubsystem->GetAllBallRows(CandidateBallIds, BallRows) ||
        CandidateBallIds.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShellGame] No Ball reward candidates. Falling back to Gold."));
        PrepareGoldReward();
        return CurrentReward.IsValid();
    }

    while (!CandidateBallIds.IsEmpty())
    {
        const int32 CandidateIndex =
            FMath::RandRange(0, CandidateBallIds.Num() - 1);
        const FName CandidateBallId = CandidateBallIds[CandidateIndex];
        CandidateBallIds.RemoveAtSwap(CandidateIndex);

        UTexture2D* BallIcon = ResolveBallRewardIcon(CandidateBallId);
        if (!BallIcon)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("[ShellGame] Ball reward has no loadable icon. BallId=%s"),
                *CandidateBallId.ToString());
            continue;
        }

        CurrentReward.Type = EPBShellGameRewardType::Ball;
        CurrentReward.BallId = CandidateBallId;
        CurrentReward.GoldAmount = 0;
        CurrentRewardIcon = BallIcon;

        UE_LOG(
            LogTemp,
            Log,
            TEXT("[ShellGame] Ball reward prepared. BallId=%s"),
            *CurrentReward.BallId.ToString());
        return true;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[ShellGame] No Ball reward with a valid icon. Falling back to Gold."));
    PrepareGoldReward();
    return CurrentReward.IsValid();
}

void APBShellGameActor::PrepareGoldReward()
{
    CurrentReward.Type = EPBShellGameRewardType::Gold;
    CurrentReward.BallId = NAME_None;
    CurrentReward.GoldAmount = FMath::Max(GoldRewardAmount, 0);
    CurrentRewardIcon = GoldRewardIcon.LoadSynchronous();

    if (!CurrentRewardIcon)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShellGame] GoldRewardIcon is not assigned or could not be loaded."));
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[ShellGame] Gold reward prepared. Amount=%d"),
        CurrentReward.GoldAmount);
}

UTexture2D* APBShellGameActor::ResolveBallRewardIcon(FName BallId) const
{
    if (BallId.IsNone())
    {
        return nullptr;
    }

    FAssetData BallAssetData;
    const FPrimaryAssetId BallAssetId(
        PBBallAssetIds::Type::BallData,
        BallId);
    if (!UAssetManager::Get().GetPrimaryAssetData(BallAssetId, BallAssetData))
    {
        return nullptr;
    }

    const UPBBallDataAsset* BallDataAsset =
        Cast<UPBBallDataAsset>(BallAssetData.GetAsset());
    return BallDataAsset
        ? BallDataAsset->BallSprite.LoadSynchronous()
        : nullptr;
}

void APBShellGameActor::ApplyRewardVisual()
{
    if (!Cups.IsValidIndex(WinningCupIndex) ||
        !IsValid(Cups[WinningCupIndex]))
    {
        return;
    }

    APBShellCupActor* WinningCup = Cups[WinningCupIndex];
    WinningCup->SetPrizeSprite(CurrentRewardIcon);
    WinningCup->SetPrizeScale(
        CurrentReward.Type == EPBShellGameRewardType::Gold
            ? GoldRewardBillboardScale
            : BallRewardBillboardScale);
}

bool APBShellGameActor::GrantCurrentReward()
{
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance || !CurrentReward.IsValid())
    {
        return false;
    }

    if (CurrentReward.Type == EPBShellGameRewardType::Ball)
    {
        UPBBallDeckSubsystem* DeckSubsystem =
            GameInstance->GetSubsystem<UPBBallDeckSubsystem>();
        if (!DeckSubsystem)
        {
            return false;
        }

        if (DeckSubsystem->AddNewBallToDeck(CurrentReward.BallId))
        {
            DeckSubsystem->LoadPlacedBallUIAssetsAsync(FStreamableDelegate());
            DeckSubsystem->LoadPlacedBallGameplayAssetsAsync(FStreamableDelegate());
            return true;
        }

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShellGame] Ball reward grant failed. Converting reward to Gold. BallId=%s"),
            *CurrentReward.BallId.ToString());
        PrepareGoldReward();
        ApplyRewardVisual();
    }

    if (CurrentReward.Type == EPBShellGameRewardType::Gold)
    {
        UPBPlayerDataSubsystem* PlayerDataSubsystem =
            GameInstance->GetSubsystem<UPBPlayerDataSubsystem>();
        if (!PlayerDataSubsystem)
        {
            return false;
        }

        PlayerDataSubsystem->GainGold(CurrentReward.GoldAmount);
        return true;
    }

    return false;
}


void APBShellGameActor::UpdateCupSlotLocations()
{
    const int32 SlotCount = CupSlots.Num();

    if (SlotCount <= 0)
    {
        return;
    }

    const float CenterIndex =
        static_cast<float>(SlotCount - 1) * 0.5f;

    for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
    {
        if (!IsValid(CupSlots[SlotIndex]))
        {
            continue;
        }

        const float Offset =
            (static_cast<float>(SlotIndex) - CenterIndex)
            * CupSpacing;

        CupSlots[SlotIndex]->SetRelativeLocation(
            FVector(0.f, Offset, 0.f));
    }
}

void APBShellGameActor::StartShellGame()
{
    if (CurrentState != EPBShellGameState::Idle &&
        CurrentState != EPBShellGameState::Finished)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShellGame] Cannot start game. CurrentState=%d"),
            static_cast<int32>(CurrentState));

        return;
    }

    if (Cups.Num() != 3 || CupSlots.Num() != 3)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShellGame] Need exactly 3 valid cups and slots."));

        return;
    }

    ResetShellGame();

    WinningCupIndex =
        FMath::RandRange(0, Cups.Num() - 1);

    if (!PrepareReward())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[ShellGame] Reward preparation failed."));
        CurrentState = EPBShellGameState::Finished;
        return;
    }

    ApplyRewardVisual();

    BuildShuffleCommands();

    if (ShuffleCommands.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShellGame] No shuffle commands were generated."));

        CurrentState = EPBShellGameState::Finished;
        return;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[ShellGame] Started. WinningCupIndex=%d"),
        WinningCupIndex);

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        return;	
    }
    
    PC->bShowMouseCursor = true;
    PC->SetInputMode(FInputModeGameAndUI());
    PC->bEnableClickEvents = true;
    PC->bEnableMouseOverEvents = true;
    
    StartShowingBall();
}

void APBShellGameActor::HandleCupSelected(int32 CupIndex)
{
    if (CurrentState != EPBShellGameState::WaitingForChoice)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShellGame] Cup selection ignored. CurrentState=%d"),
            static_cast<int32>(CurrentState));

        return;
    }

    if (!Cups.IsValidIndex(CupIndex) ||
        !Cups.IsValidIndex(WinningCupIndex))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShellGame] Invalid selected or winning cup. Selected=%d Winning=%d"),
            CupIndex,
            WinningCupIndex);

        return;
    }

    CurrentState = EPBShellGameState::Revealing;
    PopShellGameWidget();

    for (APBShellCupActor* Cup : Cups)
    {
        if (IsValid(Cup))
        {
            Cup->SetSelectionEnabled(false);
        }
    }

    const bool bCorrect = CupIndex == WinningCupIndex;
    PendingRevealCupAnimationCount = bCorrect ? 1 : 2;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[ShellGame] Selected=%d Winning=%d Result=%s"),
        CupIndex,
        WinningCupIndex,
        bCorrect ? TEXT("Success") : TEXT("Fail"));

    if (UPBSoundSubsystem* SoundSubsystem = UPBSoundSubsystem::Get(this))
    {
        SoundSubsystem->PlaySFX(
            ResultRevealSound,
            ResultRevealSoundVolume);
    }

    // 플레이어가 선택한 컵을 올린다.
    Cups[CupIndex]->RaiseCup(RevealHeight);
    
    // 오답이면 정답 컵도 함께 보여준다.
    if (!bCorrect)
    {
        Cups[WinningCupIndex]->RaiseCup(RevealHeight);
    }

    Cups[WinningCupIndex]->ShowPrize();

    if (bCorrect)
    {
        HandleSuccess();
    }
    else
    {
        HandleFailure();
    }

}

void APBShellGameActor::BuildShuffleCommands()
{
    ShuffleCommands.Reset();

    if (Cups.Num() < 2 || ShuffleCount <= 0)
    {
        return;
    }

    int32 PreviousFirst = INDEX_NONE;
    int32 PreviousSecond = INDEX_NONE;

    for (int32 ShuffleIndex = 0;
         ShuffleIndex < ShuffleCount;
         ++ShuffleIndex)
    {
        int32 FirstIndex = INDEX_NONE;
        int32 SecondIndex = INDEX_NONE;

        int32 TryCount = 0;
        constexpr int32 MaxTryCount = 20;

        do
        {
            FirstIndex = FMath::RandRange(0, Cups.Num() - 1);

            do
            {
                SecondIndex = FMath::RandRange(0, Cups.Num() - 1);
            }
            while (SecondIndex == FirstIndex);

            ++TryCount;
        }
        while (
            IsSameCupPair(
                FirstIndex,
                SecondIndex,
                PreviousFirst,
                PreviousSecond)
            && TryCount < MaxTryCount
        );

        const float DurationAlpha =
            ShuffleCount > 1
                ? static_cast<float>(ShuffleIndex) /
                  static_cast<float>(ShuffleCount - 1)
                : 1.f;

        FPBShellShuffleCommand Command;
        Command.FirstCupIndex = FirstIndex;
        Command.SecondCupIndex = SecondIndex;
        Command.Duration = FMath::Lerp(
            InitialShuffleDuration,
            MinimumShuffleDuration,
            DurationAlpha);

        ShuffleCommands.Add(Command);

        PreviousFirst = FirstIndex;
        PreviousSecond = SecondIndex;
    }
}

bool APBShellGameActor::IsSameCupPair(
    int32 FirstA,
    int32 SecondA,
    int32 FirstB,
    int32 SecondB) const
{
    return
        (FirstA == FirstB && SecondA == SecondB) ||
        (FirstA == SecondB && SecondA == FirstB);
}

void APBShellGameActor::StartNextShuffle()
{
    UE_LOG(
         LogTemp,
         Warning,
         TEXT("[ShellGame] "));
    
    if (CurrentState != EPBShellGameState::Shuffling)
    {
        return;
    }

    if (!ShuffleCommands.IsValidIndex(CurrentShuffleCommandIndex))
    {
        FinishAllShuffles();
        return;
    }

    BeginShuffleCommand(
        ShuffleCommands[CurrentShuffleCommandIndex]);
}

void APBShellGameActor::BeginShuffleCommand(
    const FPBShellShuffleCommand& Command)
{
    if (!Cups.IsValidIndex(Command.FirstCupIndex) ||
        !Cups.IsValidIndex(Command.SecondCupIndex))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShellGame] Invalid shuffle command. First=%d Second=%d"),
            Command.FirstCupIndex,
            Command.SecondCupIndex);

        FinishAllShuffles();
        return;
    }

    FirstMovingCupIndex = Command.FirstCupIndex;
    SecondMovingCupIndex = Command.SecondCupIndex;

    if (!CupSlotIndices.IsValidIndex(FirstMovingCupIndex) ||
        !CupSlotIndices.IsValidIndex(SecondMovingCupIndex))
    {
        FinishAllShuffles();
        return;
    }

    const int32 FirstSlotIndex =
        CupSlotIndices[FirstMovingCupIndex];

    const int32 SecondSlotIndex =
        CupSlotIndices[SecondMovingCupIndex];

    if (!CupSlots.IsValidIndex(FirstSlotIndex) ||
        !CupSlots.IsValidIndex(SecondSlotIndex) ||
        !IsValid(CupSlots[FirstSlotIndex]) ||
        !IsValid(CupSlots[SecondSlotIndex]))
    {
        FinishAllShuffles();
        return;
    }

    FirstStartLocation =
        Cups[FirstMovingCupIndex]->GetActorLocation();

    SecondStartLocation =
        Cups[SecondMovingCupIndex]->GetActorLocation();

    FirstTargetLocation =
        CupSlots[SecondSlotIndex]->GetComponentLocation();

    SecondTargetLocation =
        CupSlots[FirstSlotIndex]->GetComponentLocation();

    CurrentShuffleElapsed = 0.f;
    CurrentShuffleDuration =
        FMath::Max(Command.Duration, KINDA_SMALL_NUMBER);

    if (UPBSoundSubsystem* SoundSubsystem = UPBSoundSubsystem::Get(this))
    {
        const float ShuffleProgress = ShuffleCommands.Num() > 1
            ? static_cast<float>(CurrentShuffleCommandIndex)
                / static_cast<float>(ShuffleCommands.Num() - 1)
            : 0.f;
        const float SafeStartPitch = FMath::Max(ShuffleSoundStartPitch, 0.01f);
        const float SafeEndPitch = FMath::Max(ShuffleSoundEndPitch, 0.01f);
        const float Pitch = FMath::Lerp(
            SafeStartPitch,
            SafeEndPitch,
            FMath::Clamp(ShuffleProgress, 0.f, 1.f));

        SoundSubsystem->PlaySFX(
            ShuffleSound,
            ShuffleSoundVolume,
            Pitch);
    }
}

void APBShellGameActor::UpdateCurrentShuffle(float DeltaTime)
{
    if (!Cups.IsValidIndex(FirstMovingCupIndex) ||
        !Cups.IsValidIndex(SecondMovingCupIndex))
    {
        FinishAllShuffles();
        return;
    }

    CurrentShuffleElapsed += DeltaTime;

    const float RawAlpha =
        CurrentShuffleDuration > 0.f
            ? CurrentShuffleElapsed / CurrentShuffleDuration
            : 1.f;

    const float Alpha =
        FMath::Clamp(RawAlpha, 0.f, 1.f);

    const float SmoothAlpha =
        FMath::InterpEaseInOut(
            0.f,
            1.f,
            Alpha,
            ShuffleEaseExponent);

    FVector FirstLocation =
        FMath::Lerp(
            FirstStartLocation,
            FirstTargetLocation,
            SmoothAlpha);

    FVector SecondLocation =
        FMath::Lerp(
            SecondStartLocation,
            SecondTargetLocation,
            SmoothAlpha);

    const float ArcAlpha =
        FMath::Sin(SmoothAlpha * PI);

    /*
     * 첫 컵은 +X 방향,
     * 두 번째 컵은 -X 방향으로 우회한다.
     */
    FirstLocation.X += ArcAlpha * ShuffleArcDistance;
    SecondLocation.X -= ArcAlpha * ShuffleArcDistance;

    Cups[FirstMovingCupIndex]->SetActorLocation(FirstLocation);
    Cups[SecondMovingCupIndex]->SetActorLocation(SecondLocation);

    if (Alpha >= 1.f)
    {
        FinishCurrentShuffle();
    }
}

void APBShellGameActor::FinishCurrentShuffle()
{
    if (!Cups.IsValidIndex(FirstMovingCupIndex) ||
        !Cups.IsValidIndex(SecondMovingCupIndex) ||
        !CupSlotIndices.IsValidIndex(FirstMovingCupIndex) ||
        !CupSlotIndices.IsValidIndex(SecondMovingCupIndex))
    {
        FinishAllShuffles();
        return;
    }

    const int32 FirstSlotIndex =
        CupSlotIndices[FirstMovingCupIndex];

    const int32 SecondSlotIndex =
        CupSlotIndices[SecondMovingCupIndex];

    Swap(
        CupSlotIndices[FirstMovingCupIndex],
        CupSlotIndices[SecondMovingCupIndex]);

    // 프레임 누적으로 생길 수 있는 위치 오차를 제거한다.
    Cups[FirstMovingCupIndex]->SetActorLocation(
        CupSlots[SecondSlotIndex]->GetComponentLocation());

    Cups[SecondMovingCupIndex]->SetActorLocation(
        CupSlots[FirstSlotIndex]->GetComponentLocation());

    ++CurrentShuffleCommandIndex;

    StartNextShuffle();
}

void APBShellGameActor::FinishAllShuffles()
{
    CurrentState = EPBShellGameState::WaitingForChoice;
    PushShellGameWidget();

    for (APBShellCupActor* Cup : Cups)
    {
        if (IsValid(Cup))
        {
            Cup->SetSelectionEnabled(true);
        }
    }

    FirstMovingCupIndex = INDEX_NONE;
    SecondMovingCupIndex = INDEX_NONE;

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[ShellGame] Shuffle finished. Waiting for player choice."));
}

void APBShellGameActor::OpenAbility()
{
    StartShellGame();
    UE_LOG(
       LogTemp,
       Warning,
       TEXT("[ShellGame open] "));
}

void APBShellGameActor::StartShowingBall()
{
    if (!Cups.IsValidIndex(WinningCupIndex))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShellGame] StartShowingBall failed. WinningCupIndex=%d"),
            WinningCupIndex);

        CurrentState = EPBShellGameState::Finished;
        return;
    }

    CurrentState = EPBShellGameState::ShowingBall;

    APBShellCupActor* WinningCup =
        Cups[WinningCupIndex];

    WinningCup->ShowPrize();
    WinningCup->RaiseCup(RevealHeight);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[ShellGame] Showing ball. WinningCupIndex=%d"),
        WinningCupIndex);
}

void APBShellGameActor::FinishShowingBall()
{
    if (CurrentState != EPBShellGameState::ShowingBall)
    {
        return;
    }

    StartCoveringBall();
}

void APBShellGameActor::StartCoveringBall()
{
    if (!Cups.IsValidIndex(WinningCupIndex))
    {
        CurrentState = EPBShellGameState::Finished;
        return;
    }

    CurrentState = EPBShellGameState::CoveringBall;

    APBShellCupActor* WinningCup =
        Cups[WinningCupIndex];

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[ShellGame] Covering ball."));

    WinningCup->LowerCup();
}

void APBShellGameActor::FinishCoveringBall()
{
    if (CurrentState != EPBShellGameState::CoveringBall)
    {
        return;
    }

    if (!Cups.IsValidIndex(WinningCupIndex))
    {
        CurrentState = EPBShellGameState::Finished;
        return;
    }

    APBShellCupActor* WinningCup =
        Cups[WinningCupIndex];

    // 컵으로 완전히 덮은 뒤 공을 숨긴다.
    WinningCup->HidePrize();

    StartShuffling();
}

void APBShellGameActor::HandleCupRaiseFinished(APBShellCupActor* Cup)
{
    if (!IsValid(Cup))
    {
        return;
    }

    if (CurrentState == EPBShellGameState::ShowingBall)
    {
        if (!Cups.IsValidIndex(WinningCupIndex) ||
            Cup != Cups[WinningCupIndex])
        {
            return;
        }

        GetWorldTimerManager().SetTimer(
            ShowBallTimerHandle,
            this,
            &APBShellGameActor::FinishShowingBall,
            ShowBallDuration,
            false);
        return;
    }

    if (CurrentState != EPBShellGameState::Revealing ||
        PendingRevealCupAnimationCount <= 0)
    {
        return;
    }

    --PendingRevealCupAnimationCount;

    if (PendingRevealCupAnimationCount == 0)
    {
        StartRevealHoldTimer();
    }
}

void APBShellGameActor::HandleCupLowerFinished(APBShellCupActor* Cup)
{
    if (CurrentState != EPBShellGameState::CoveringBall ||
        !Cups.IsValidIndex(WinningCupIndex) ||
        Cup != Cups[WinningCupIndex])
    {
        return;
    }

    FinishCoveringBall();
}

void APBShellGameActor::StartRevealHoldTimer()
{
    GetWorldTimerManager().SetTimer(
        RevealTimerHandle,
        this,
        &APBShellGameActor::FinishReveal,
        RevealDuration,
        false);
}

void APBShellGameActor::StartShuffling()
{
    CurrentShuffleCommandIndex = 0;

    FirstMovingCupIndex = INDEX_NONE;
    SecondMovingCupIndex = INDEX_NONE;

    CurrentShuffleElapsed = 0.f;
    CurrentShuffleDuration = 0.f;

    CurrentState = EPBShellGameState::Shuffling;

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[ShellGame] Shuffle started. CommandCount=%d"),
        ShuffleCommands.Num());

    StartNextShuffle();
}


void APBShellGameActor::HandleSuccess()
{
    bRewardGranted = GrantCurrentReward();

    if (!bRewardGranted)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[ShellGame] Correct cup selected, but reward grant failed."));
        return;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[ShellGame] Correct cup selected. RewardGranted=%s RewardType=%d BallId=%s Gold=%d"),
        bRewardGranted ? TEXT("true") : TEXT("false"),
        static_cast<int32>(CurrentReward.Type),
        *CurrentReward.BallId.ToString(),
        CurrentReward.GoldAmount);
}

void APBShellGameActor::HandleFailure()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[ShellGame] Wrong cup selected."));

    // TODO:
    // 베팅 골드 차감
    // 실패 연출
}

void APBShellGameActor::FinishReveal()
{
    CurrentState = EPBShellGameState::WaitingForRewardConfirmation;
    ShowRewardPopup();
}

FText APBShellGameActor::BuildRewardPopupMessage() const
{
    if (!bRewardGranted)
    {
        return NSLOCTEXT(
            "ShellGame",
            "RewardFailedPopup",
            "보상 획득에 실패했습니다.");
    }

    if (CurrentReward.Type == EPBShellGameRewardType::Ball)
    {
        FText BallDisplayName = FText::FromName(CurrentReward.BallId);

        const UGameInstance* GameInstance = GetGameInstance();
        const UPBTableDataSubsystem* TableDataSubsystem = GameInstance
            ? GameInstance->GetSubsystem<UPBTableDataSubsystem>()
            : nullptr;
        if (TableDataSubsystem)
        {
            TArray<FName> BallIds;
            TArray<FPBBallTableRow> BallRows;
            if (TableDataSubsystem->GetAllBallRows(BallIds, BallRows))
            {
                const int32 BallIndex = BallIds.IndexOfByKey(CurrentReward.BallId);
                if (BallRows.IsValidIndex(BallIndex) &&
                    !BallRows[BallIndex].DisplayName.IsEmpty())
                {
                    BallDisplayName = BallRows[BallIndex].DisplayName;
                }
            }
        }

        return FText::Format(
            NSLOCTEXT("ShellGame", "BallRewardPopup", "{0} 볼을 획득했습니다."),
            BallDisplayName);
    }

    if (CurrentReward.Type == EPBShellGameRewardType::Gold)
    {
        const UGameInstance* GameInstance = GetGameInstance();
        const UPBPlayerDataSubsystem* PlayerDataSubsystem = GameInstance
            ? GameInstance->GetSubsystem<UPBPlayerDataSubsystem>()
            : nullptr;

        return FText::Format(
            NSLOCTEXT(
                "ShellGame",
                "GoldRewardPopupWithCurrentGold",
                "{0}G 획득했습니다.\n현재 골드: {1}G"),
            FText::AsNumber(CurrentReward.GoldAmount),
            FText::AsNumber(PlayerDataSubsystem->GetCurrentGold()));
    }

    return NSLOCTEXT("ShellGame", "UnknownRewardPopup", "보상을 획득했습니다.");
}

void APBShellGameActor::ShowRewardPopup()
{
    UGameInstance* GameInstance = GetGameInstance();
    UPBUIManagerSubsystem* UIManagerSubsystem = GameInstance
        ? GameInstance->GetSubsystem<UPBUIManagerSubsystem>()
        : nullptr;
    if (!UIManagerSubsystem)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[ShellGame] UIManagerSubsystem is unavailable. Skipping reward confirmation."));
        CompleteShellGame();
        return;
    }

    if (bRewardGranted &&
        CurrentReward.Type == EPBShellGameRewardType::Ball)
    {
        if (UIManagerSubsystem->ShowBallRewardPopup(
            BuildRewardPopupMessage(),
            CurrentReward.BallId,
            1,
            FPBSimplePopupClosedDelegate::CreateUObject(
                this,
                &APBShellGameActor::HandleRewardPopupClosed)))
        {
            return;
        }

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShellGame] Failed to show the Ball reward popup. Falling back to the simple popup."));
    }

    if (!UIManagerSubsystem->ShowSimplePopup(
        BuildRewardPopupMessage(),
        FPBSimplePopupClosedDelegate::CreateUObject(
            this,
            &APBShellGameActor::HandleRewardPopupClosed)))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[ShellGame] Failed to create the reward popup. Skipping reward confirmation."));
        CompleteShellGame();
    }
}

void APBShellGameActor::HandleRewardPopupClosed(const bool bConfirmed)
{
    if (CurrentState != EPBShellGameState::WaitingForRewardConfirmation)
    {
        return;
    }

    if (!bConfirmed)
    {
        ShowRewardPopup();
        return;
    }

    CompleteShellGame();
}

void APBShellGameActor::PushShellGameWidget()
{
    if (!ShellGameWidgetClass)
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UPBUIManagerSubsystem* UIManagerSubsystem = GameInstance
        ? GameInstance->GetSubsystem<UPBUIManagerSubsystem>()
        : nullptr;
    if (!UIManagerSubsystem)
    {
        return;
    }

    ShellGameWidget = UIManagerSubsystem->PushWidget(
        ShellGameWidgetClass,
        0);
}

void APBShellGameActor::PopShellGameWidget()
{
    if (IsValid(ShellGameWidget))
    {
        ShellGameWidget->CompletePop();
    }

    ShellGameWidget = nullptr;
}

void APBShellGameActor::CompleteShellGame()
{
    CurrentState = EPBShellGameState::Finished;
    PopShellGameWidget();

    FPBChoiceType Message;
    Message.Exit = 1;
    UE_LOG(
        LogTemp,
        Log,
        TEXT("[ShellGame] Reveal finished."));

    UGameplayMessageSubsystem::Get(this).BroadcastMessage(
        GameplayTags::Event_UI_Choice_Exit,Message);
    
  
}
