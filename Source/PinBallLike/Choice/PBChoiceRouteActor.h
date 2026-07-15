#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBChoiceNodeTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Struct/Choice/PBChoiceType.h"
#include "Components/SplineMeshComponent.h"
#include "PBChoiceRouteActor.generated.h"

class USceneComponent;
class USplineComponent;
class USplineMeshComponent;
class UStaticMesh;
class UMaterialInterface;

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
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void BuildSplineMeshes(
        USplineComponent* TargetSpline,
        TArray<TObjectPtr<USplineMeshComponent>>& OutSplineMeshes,
        const FString& ComponentNamePrefix);

    void ClearSplineMeshes(
        TArray<TObjectPtr<USplineMeshComponent>>& SplineMeshes);

    void StartMove(
        USplineComponent* TargetSpline,
        EPBChoiceRouteSide RouteSide);

    void MoveToNextPoint();
    void ApplyBallTransform(float DeltaTime);
    void ArriveAtPoint(int32 PointIndex);

    void OnReachPoint(EPBChoiceNodeType NodeType);
    void FinishMove();

    void FadeToNodeDestination(EPBChoiceNodeType NodeType);
    void OnFadeToDestinationFinished();

    void HandleExitStart(
        FGameplayTag Exit,
        const FPBChoiceType& Message);

    void FadeBackToBallAndMove();
    void OnFadeBackToBallFinished();

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, Category = "Choice Route|Spline")
    TObjectPtr<USplineComponent> LeftSpline;

    UPROPERTY(VisibleAnywhere, Category = "Choice Route|Spline")
    TObjectPtr<USplineComponent> RightSpline;

    /*
     * 스플라인 구간에 휘어질 원본 메시입니다.
     * 메시가 X축 방향으로 길게 만들어졌다면 ForwardAxis를 X로 설정합니다.
     */
    UPROPERTY(EditAnywhere, Category = "Choice Route|Spline Mesh")
    TObjectPtr<UStaticMesh> RouteSplineMesh;

    UPROPERTY(EditAnywhere, Category = "Choice Route|Spline Mesh")
    TObjectPtr<UMaterialInterface> RouteSplineMaterial;

    UPROPERTY(EditAnywhere, Category = "Choice Route|Spline Mesh")
    TEnumAsByte<ESplineMeshAxis::Type> SplineMeshForwardAxis =
        ESplineMeshAxis::X;

    UPROPERTY(EditAnywhere, Category = "Choice Route|Spline Mesh")
    FVector2D SplineMeshScale = FVector2D(1.f, 1.f);

    UPROPERTY(EditAnywhere, Category = "Choice Route|Spline Mesh")
    bool bSplineMeshCollisionEnabled = false;

    UPROPERTY(EditAnywhere, Category = "Choice Route|Spline Mesh")
    bool bSmoothInterpRollScale = true;
    /*
     * OnConstruction에서 생성한 컴포넌트를 추적하기 위한 배열입니다.
     */
    UPROPERTY(Transient)
    TArray<TObjectPtr<USplineMeshComponent>> LeftSplineMeshes;

    UPROPERTY(Transient)
    TArray<TObjectPtr<USplineMeshComponent>> RightSplineMeshes;

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

    UPROPERTY(EditAnywhere, Category = "Choice Route|Destination")
    TMap<EPBChoiceNodeType, FPBChoiceNodeDestination> NodeDestinations;

    EPBChoiceNodeType PendingNodeType = EPBChoiceNodeType::None;

    UPROPERTY()
    TObjectPtr<AActor> PendingCameraActor;

    UPROPERTY()
    TObjectPtr<AActor> PendingActionActor;

    UPROPERTY()
    TObjectPtr<USplineComponent> CurrentSpline;

    UPROPERTY()
    TObjectPtr<UPBChoiceWidget> ChoiceWidget;

    UPROPERTY(EditAnywhere, Category = "Choice Route|UI")
    TSubclassOf<UPBChoiceWidget> ChoiceWidgetClass;

    EPBChoiceRouteSide CurrentRouteSide = EPBChoiceRouteSide::Left;

    int32 CurrentPointIndex = 0;
    int32 TargetPointIndex = 1;

    float CurrentDistance = 0.f;
    float TargetDistance = 0.f;

    bool bMoving = false;

    FGameplayMessageListenerHandle ExitStartHandle;

    FTimerHandle FadeToDestinationTimerHandle;
    FTimerHandle FadeBackTimerHandle;
};