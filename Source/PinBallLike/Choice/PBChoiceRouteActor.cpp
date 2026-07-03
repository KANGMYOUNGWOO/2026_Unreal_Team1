#include "PBChoiceRouteActor.h"

#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"

#include "PBChoiceBallActor.h"
#include "PBChoiceNodeManager.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Shop/MVVM/PBShopActor.h"
#include  "UI/PBChoiceWidget.h"

APBChoiceRouteActor::APBChoiceRouteActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    LeftSpline = CreateDefaultSubobject<USplineComponent>(TEXT("LeftSpline"));
    LeftSpline->SetupAttachment(Root);

    RightSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RightSpline"));
    RightSpline->SetupAttachment(Root);
}

void APBChoiceRouteActor::BeginPlay()
{
    Super::BeginPlay();

    if (NodeManager)
    {
        NodeManager->BuildNodes(LeftSpline, RightSpline);
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (OverviewCameraActor)
        {
            
            ChoiceWidget = CreateWidget<UPBChoiceWidget>(PC, ChoiceWidgetClass);
            ChoiceWidget->SetChoiceRouteActor(this);
            ChoiceWidget->AddToViewport();
            
            PC->SetViewTarget(OverviewCameraActor);
            
            
        }
    }
}

void APBChoiceRouteActor::ChooseLeft()
{
    StartMove(LeftSpline, EPBChoiceRouteSide::Left);
}

void APBChoiceRouteActor::ChooseRight()
{
    StartMove(RightSpline, EPBChoiceRouteSide::Right);
}

void APBChoiceRouteActor::StartMove(
    USplineComponent* TargetSpline,
    EPBChoiceRouteSide RouteSide)
{
    if (!TargetSpline || !ChoiceBallActor)
    {
        return;
    }

    const int32 PointCount = TargetSpline->GetNumberOfSplinePoints();

    if (PointCount < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Spline needs at least 2 points."));
        return;
    }

    CurrentSpline = TargetSpline;
    CurrentRouteSide = RouteSide;

    CurrentPointIndex = 0;
    TargetPointIndex = 1;

    CurrentDistance =
        CurrentSpline->GetDistanceAlongSplineAtSplinePoint(CurrentPointIndex);

    TargetDistance =
        CurrentSpline->GetDistanceAlongSplineAtSplinePoint(TargetPointIndex);

    ApplyBallTransform(0.f);

    bMoving = true;

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PC->SetViewTargetWithBlend(
            ChoiceBallActor,
            BallCameraBlendTime);
    }
}

void APBChoiceRouteActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bMoving || !CurrentSpline || !ChoiceBallActor)
    {
        return;
    }

    CurrentDistance += MoveSpeed * DeltaTime;

    if (CurrentDistance >= TargetDistance)
    {
        CurrentDistance = TargetDistance;

        ApplyBallTransform(DeltaTime);

        ArriveAtPoint(TargetPointIndex);
        return;
    }

    ApplyBallTransform(DeltaTime);
}

void APBChoiceRouteActor::ApplyBallTransform(float DeltaTime)
{
    if (!CurrentSpline || !ChoiceBallActor)
    {
        return;
    }

    const FVector NewLocation =
        CurrentSpline->GetLocationAtDistanceAlongSpline(
            CurrentDistance,
            ESplineCoordinateSpace::World);

    const FRotator NewRotation =
        CurrentSpline->GetRotationAtDistanceAlongSpline(
            CurrentDistance,
            ESplineCoordinateSpace::World);

    ChoiceBallActor->ApplyRouteTransform(
        NewLocation,
        NewRotation,
        DeltaTime);
}

void APBChoiceRouteActor::ArriveAtPoint(int32 PointIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("Arrived Point %d"), PointIndex);

    CurrentPointIndex = PointIndex;
    bMoving = false;

    EPBChoiceNodeType NodeType = EPBChoiceNodeType::None;

    if (NodeManager)
    {
        NodeType =
            NodeManager->HandleBallArrivedAtPoint(
                CurrentRouteSide,
                PointIndex);
    }

    OnReachPoint(NodeType);
}

void APBChoiceRouteActor::OnReachPoint(EPBChoiceNodeType NodeType)
{
    switch (NodeType)
    {
    case EPBChoiceNodeType::Shop:
        FadeToShop();
        break;

    case EPBChoiceNodeType::Enhance:
        UE_LOG(LogTemp, Warning, TEXT("Enhance Node Reached"));
        MoveToNextPoint();
        break;

    case EPBChoiceNodeType::Combine:
        UE_LOG(LogTemp, Warning, TEXT("Combine Node Reached"));
        MoveToNextPoint();
        break;

    case EPBChoiceNodeType::Battle:
        UE_LOG(LogTemp, Warning, TEXT("Battle Node Reached"));
        MoveToNextPoint();
        break;

    case EPBChoiceNodeType::Event:
        UE_LOG(LogTemp, Warning, TEXT("Event Node Reached"));
        MoveToNextPoint();
        break;

    case EPBChoiceNodeType::Boss:
        UE_LOG(LogTemp, Warning, TEXT("Boss Node Reached"));
        FinishMove();
        break;

    case EPBChoiceNodeType::None:
    default:
        MoveToNextPoint();
        break;
    }
}

void APBChoiceRouteActor::MoveToNextPoint()
{
    if (!CurrentSpline)
    {
        return;
    }

    const int32 LastPointIndex =
        CurrentSpline->GetNumberOfSplinePoints() - 1;

    if (CurrentPointIndex >= LastPointIndex)
    {
        FinishMove();
        return;
    }

    TargetPointIndex = CurrentPointIndex + 1;

    TargetDistance =
        CurrentSpline->GetDistanceAlongSplineAtSplinePoint(TargetPointIndex);

    bMoving = true;
}

void APBChoiceRouteActor::FinishMove()
{
    bMoving = false;

    UE_LOG(LogTemp, Warning, TEXT("Route Finished"));
}

void APBChoiceRouteActor::FadeToShop()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

    if (!PC || !PC->PlayerCameraManager)
    {
        return;
    }

    PC->PlayerCameraManager->StartCameraFade(
        0.f,
        1.f,
        FadeOutTime,
        FLinearColor::Black,
        false,
        true);

    GetWorld()->GetTimerManager().SetTimer(
        FadeTimerHandle,
        this,
        &APBChoiceRouteActor::OnFadeOutFinished,
        FadeOutTime,
        false);
}

void APBChoiceRouteActor::OnFadeOutFinished()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

    if (!PC || !PC->PlayerCameraManager)
    {
        return;
    }

    if (ShopCameraActor)
    {
        PC->SetViewTarget(ShopCameraActor);
    }

    if (ShopActor)
    {
        ShopActor->OpenShop();
    }

    PC->PlayerCameraManager->StartCameraFade(
        1.f,
        0.f,
        FadeInTime,
        FLinearColor::Black,
        false,
        false);
}