#include "PBShellGameActor.h"

#include "PBShellCupActor.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

APBShellGameActor::APBShellGameActor()
{
    PrimaryActorTick.bCanEverTick = true;

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

    ShuffleCommands.Reset();

    CupSlotIndices = { 0, 1, 2 };

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

        // 컵 안의 공도 숨김
        Cups[CupIndex]->HidePrize();
    }
}

void APBShellGameActor::StartShellGame()
{
    if (CurrentState == EPBShellGameState::Shuffling)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShellGame] Shuffle is already running."));

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

    WinningCupIndex = FMath::RandRange(0, Cups.Num() - 1);

    /*
     * 지금은 공을 잠시 보여주는 연출 없이 바로 섞는다.
     * 나중에는 ShowingBall 상태와 Timer를 추가해서
     * 정답 컵 밑의 공을 보여준 후 셔플을 시작할 수 있다.
     */
    BuildShuffleCommands();

    if (ShuffleCommands.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[ShellGame] No shuffle commands were generated."));

        CurrentState = EPBShellGameState::WaitingForChoice;
        return;
    }

    CurrentShuffleCommandIndex = 0;
    CurrentState = EPBShellGameState::Shuffling;

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[ShellGame] Started. WinningCupIndex=%d ShuffleCount=%d"),
        WinningCupIndex,
        ShuffleCommands.Num());

    StartNextShuffle();
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

    const bool bCorrect = CupIndex == WinningCupIndex;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[ShellGame] Selected=%d Winning=%d Result=%s"),
        CupIndex,
        WinningCupIndex,
        bCorrect ? TEXT("Success") : TEXT("Fail"));

    // 플레이어가 선택한 컵을 올린다.
    Cups[CupIndex]->RaiseCup(RevealHeight);

    // 오답이면 정답 컵도 함께 보여준다.
    if (!bCorrect)
    {
        Cups[WinningCupIndex]->RaiseCup(RevealHeight);
    }

 

    if (bCorrect)
    {
        HandleSuccess();
    }
    else
    {
        HandleFailure();
    }

    GetWorldTimerManager().SetTimer(
        RevealTimerHandle,
        this,
        &APBShellGameActor::FinishReveal,
        RevealDuration,
        false);
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

    FirstMovingCupIndex = INDEX_NONE;
    SecondMovingCupIndex = INDEX_NONE;

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[ShellGame] Shuffle finished. Waiting for player choice."));
}


void APBShellGameActor::HandleSuccess()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[ShellGame] Correct cup selected."));

    // TODO:
    // 골드 지급
    // 볼 지급
    // 강화권 지급
    // PlayerDataSubsystem 갱신
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
    CurrentState = EPBShellGameState::Finished;

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[ShellGame] Reveal finished."));

    /*
     * 여기서 선택지 종료 메시지를 발행하거나
     * PBChoiceRouteActor로 돌아가는 처리를 하면 된다.
     */
}