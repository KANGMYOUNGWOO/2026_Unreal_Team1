#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBChoiceNodeTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Struct/Choice/PBChoiceType.h"
#include "PBChoiceRouteActor.generated.h"

class USceneComponent;
class USplineComponent;
class APBChoiceBallActor;
class APBChoiceNodeManager;
class APBShopActor;
class UPBChoiceWidget;

USTRUCT(BlueprintType)
struct FPBChoiceNodeDestination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    TObjectPtr<AActor> CameraActor = nullptr;

    UPROPERTY(EditAnywhere)
    TObjectPtr<AActor> ActionActor = nullptr;
};


UCLASS()
class PINBALLLIKE_API APBChoiceRouteActor : public AActor
{
    GENERATED_BODY()

public:
    APBChoiceRouteActor();

    UFUNCTION(BlueprintCallable)
    void ChooseLeft();

    UFUNCTION(BlueprintCallable)
    void ChooseRight();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void StartMove(USplineComponent* TargetSpline, EPBChoiceRouteSide RouteSide);
    void MoveToNextPoint();

    void ApplyBallTransform(float DeltaTime);
    void ArriveAtPoint(int32 PointIndex);

    void OnReachPoint(EPBChoiceNodeType NodeType);
    void FinishMove();

    void FadeToShop();
    void OnFadeOutFinished();

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, Category = "Choice Route|Spline")
    TObjectPtr<USplineComponent> LeftSpline;

    UPROPERTY(VisibleAnywhere, Category = "Choice Route|Spline")
    TObjectPtr<USplineComponent> RightSpline;

    UPROPERTY(EditAnywhere, Category = "Choice Route|Ball")
    TObjectPtr<APBChoiceBallActor> ChoiceBallActor;

    UPROPERTY(EditAnywhere, Category = "Choice Route|Node")
    TObjectPtr<APBChoiceNodeManager> NodeManager;

    UPROPERTY(EditAnywhere, Category = "Choice Route|Shop")
    TObjectPtr<APBShopActor> ShopActor;

    UPROPERTY(EditAnywhere, Category = "Choice Route|Camera")
    TObjectPtr<AActor> OverviewCameraActor;
    
    UPROPERTY(EditAnywhere, Category = "Choice Route|Move")
    float MoveSpeed = 500.f;

    UPROPERTY(EditAnywhere, Category = "Choice Route|Camera")
    float BallCameraBlendTime = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Choice Route|Camera")
    float FadeOutTime = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Choice Route|Camera")
    float FadeInTime = 0.5f;
    
    UPROPERTY(EditAnywhere, Category="Choice Route|Destination")
    TMap<EPBChoiceNodeType, FPBChoiceNodeDestination> NodeDestinations;
    
    void FadeToNodeDestination(EPBChoiceNodeType NodeType);
    void OnFadeToDestinationFinished();

    EPBChoiceNodeType PendingNodeType = EPBChoiceNodeType::None;

    UPROPERTY()
    TObjectPtr<AActor> PendingCameraActor;

    UPROPERTY()
    TObjectPtr<AActor> PendingActionActor;

    FTimerHandle FadeToDestinationTimerHandle;
    
    UPROPERTY()
    TObjectPtr<USplineComponent> CurrentSpline;

    UPROPERTY()
    TObjectPtr<UPBChoiceWidget> ChoiceWidget;
    
    UPROPERTY(EditAnywhere, Category = "Choice Route|UI")
    TSubclassOf<UPBChoiceWidget> ChoiceWidgetClass;
    
    void HandleExitStart(FGameplayTag Exit, const FPBChoiceType& Message);
    void FadeBackToBallAndMove();
    void OnFadeBackToBallFinished();
    
    EPBChoiceRouteSide CurrentRouteSide = EPBChoiceRouteSide::Left;

    int32 CurrentPointIndex = 0;
    int32 TargetPointIndex = 1;

    float CurrentDistance = 0.f;
    float TargetDistance = 0.f;

    bool bMoving = false;

    FGameplayMessageListenerHandle ExitStartHandle;
    
    FTimerHandle FadeTimerHandle;
    FTimerHandle FadeBackTimerHandle; 
  

   
};