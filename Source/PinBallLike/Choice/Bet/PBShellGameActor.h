#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBShellGameActor.generated.h"

class APBShellCupActor;
class USceneComponent;

UENUM()
enum class EPBShellGameState : uint8
{
    Idle,
    ShowingBall,
    CoveringBall,
    Shuffling,
    WaitingForChoice,
    Revealing,
    Finished
};

USTRUCT()
struct FPBShellShuffleCommand
{
    GENERATED_BODY()

    int32 FirstCupIndex = INDEX_NONE;
    int32 SecondCupIndex = INDEX_NONE;
    float Duration = 0.5f;
};

UCLASS()
class PINBALLLIKE_API APBShellGameActor : public AActor
{
    GENERATED_BODY()

public:
    APBShellGameActor();

    virtual void Tick(float DeltaTime) override;

    void StartShellGame();
    void HandleCupSelected(int32 CupIndex);

private:
    void BuildShuffleCommands();
    void StartNextShuffle();
    void BeginShuffleCommand(const FPBShellShuffleCommand& Command);
    void UpdateCurrentShuffle(float DeltaTime);
    void FinishCurrentShuffle();
    void FinishAllShuffles();
protected:
    virtual void BeginPlay() override;
private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> LeftSlot;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> CenterSlot;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> RightSlot;

    UPROPERTY(EditAnywhere, Category = "Shell Game")
    TArray<TObjectPtr<APBShellCupActor>> Cups;

    UPROPERTY(EditAnywhere, Category = "Shell Game|Shuffle")
    int32 ShuffleCount = 8;

    UPROPERTY(EditAnywhere, Category = "Shell Game|Shuffle")
    float InitialShuffleDuration = 0.65f;

    UPROPERTY(EditAnywhere, Category = "Shell Game|Shuffle")
    float MinimumShuffleDuration = 0.3f;

    UPROPERTY(EditAnywhere, Category = "Shell Game|Shuffle")
    float ShuffleArcDistance = 120.f;

    UPROPERTY(EditAnywhere, Category = "Shell Game|Shuffle")
    float ShuffleEaseExponent = 2.f;

    UPROPERTY(EditAnywhere, Category = "Shell Game|Reveal")
    float RevealHeight = 150.f;

    UPROPERTY(EditAnywhere, Category = "Shell Game|Reveal")
    float RevealDuration = 2.f;

    UPROPERTY(EditAnywhere, Category = "Shell Game|Reveal")
    FVector PrizeBallOffset = FVector(0.f, 0.f, 20.f);

    UPROPERTY(EditAnywhere, Category = "Shell Game")
    bool bAutoStartOnBeginPlay = false;
    
    FTimerHandle RevealTimerHandle;
    
    TArray<TObjectPtr<USceneComponent>> CupSlots;
    TArray<int32> CupSlotIndices;
    TArray<FPBShellShuffleCommand> ShuffleCommands;

    int32 CurrentShuffleCommandIndex = 0;
    int32 FirstMovingCupIndex = INDEX_NONE;
    int32 SecondMovingCupIndex = INDEX_NONE;

    FVector FirstStartLocation;
    FVector FirstTargetLocation;
    FVector SecondStartLocation;
    FVector SecondTargetLocation;

    float CurrentShuffleElapsed = 0.f;
    float CurrentShuffleDuration = 0.f;

    EPBShellGameState CurrentState = EPBShellGameState::Idle;

    // 공이 들어 있는 컵의 Actor 인덱스
    int32 WinningCupIndex = INDEX_NONE;
    
private:
    bool InitializeCups();
    void ResetShellGame();

    bool IsSameCupPair(
        int32 FirstA,
        int32 SecondA,
        int32 FirstB,
        int32 SecondB) const;

  
    void HandleSuccess();
    void HandleFailure();
    void FinishReveal();
    
};