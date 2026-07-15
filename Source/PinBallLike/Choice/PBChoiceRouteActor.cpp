#include "PBChoiceRouteActor.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"

#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PBChoiceBallActor.h"
#include "PBChoiceNodeManager.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Shop/MVVM/PBShopActor.h"
#include "UI/PBChoiceWidget.h"

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
            
            PC->bShowMouseCursor = true;
            PC->SetInputMode(FInputModeGameAndUI());
            PC->bEnableClickEvents = true;
            PC->bEnableMouseOverEvents = true;
        }
    }
    UGameplayMessageSubsystem& MessageSubsystem =
      UGameplayMessageSubsystem::Get(this);
    
    ExitStartHandle =
        MessageSubsystem.RegisterListener<FPBChoiceType>(GameplayTags::Event_UI_Choice_Exit,
            this,
            &APBChoiceRouteActor::HandleExitStart);
}

void APBChoiceRouteActor::ChooseLeft()
{
    StartMove(LeftSpline, EPBChoiceRouteSide::Left);
}

void APBChoiceRouteActor::ChooseRight()
{
    StartMove(RightSpline, EPBChoiceRouteSide::Right);
}

void APBChoiceRouteActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    
    BuildSplineMeshes(
        LeftSpline,
        LeftSplineMeshes,
        TEXT("LeftRouteMesh"));

    BuildSplineMeshes(
        RightSpline,
        RightSplineMeshes,
        TEXT("RightRouteMesh"));
        
}

void APBChoiceRouteActor::BuildSplineMeshes(USplineComponent* TargetSpline,
    TArray<TObjectPtr<USplineMeshComponent>>& OutSplineMeshes, const FString& ComponentNamePrefix)
{
     ClearSplineMeshes(OutSplineMeshes);

    if (!IsValid(TargetSpline))
    {
        return;
    }

    if (!IsValid(RouteSplineMesh))
    {
        return;
    }

    const int32 PointCount =
        TargetSpline->GetNumberOfSplinePoints();

    if (PointCount < 2)
    {
        return;
    }

    /*
     * 포인트가 N개라면 구간은 N - 1개입니다.
     *
     * 0 ─ 1 ─ 2 ─ 3
     *   0   1   2
     */
    for (int32 PointIndex = 0;
         PointIndex < PointCount - 1;
         ++PointIndex)
    {
        const FVector StartPosition =
            TargetSpline->GetLocationAtSplinePoint(
                PointIndex,
                ESplineCoordinateSpace::Local);

        const FVector StartTangent =
            TargetSpline->GetTangentAtSplinePoint(
                PointIndex,
                ESplineCoordinateSpace::Local);

        const FVector EndPosition =
            TargetSpline->GetLocationAtSplinePoint(
                PointIndex + 1,
                ESplineCoordinateSpace::Local);

        const FVector EndTangent =
            TargetSpline->GetTangentAtSplinePoint(
                PointIndex + 1,
                ESplineCoordinateSpace::Local);

        const FName ComponentName(
            *FString::Printf(
                TEXT("%s_%d"),
                *ComponentNamePrefix,
                PointIndex));

        USplineMeshComponent* SplineMesh =
            NewObject<USplineMeshComponent>(
                this,
                USplineMeshComponent::StaticClass(),
                ComponentName,
                RF_Transactional);

        if (!IsValid(SplineMesh))
        {
            continue;
        }

        /*
         * 동적으로 생성한 컴포넌트를 액터가 소유하도록 등록합니다.
         */
        AddInstanceComponent(SplineMesh);

        /*
         * 좌표를 Local로 읽었으므로 TargetSpline의 자식으로 붙입니다.
         */
        SplineMesh->SetupAttachment(TargetSpline);

        SplineMesh->SetMobility(EComponentMobility::Movable);
        SplineMesh->SetStaticMesh(RouteSplineMesh);

        SplineMesh->SetForwardAxis(
            SplineMeshForwardAxis,
            false);

        SplineMesh->SetStartAndEnd(
            StartPosition,
            StartTangent,
            EndPosition,
            EndTangent,
            false);

        SplineMesh->SetStartScale(
            SplineMeshScale,
            false);

        SplineMesh->SetEndScale(
            SplineMeshScale,
            false);

        if (IsValid(RouteSplineMaterial))
        {
            SplineMesh->SetMaterial(
                0,
                RouteSplineMaterial);
        }

        SplineMesh->SetCollisionEnabled(
            bSplineMeshCollisionEnabled
                ? ECollisionEnabled::QueryAndPhysics
                : ECollisionEnabled::NoCollision);

        SplineMesh->RegisterComponent();
        SplineMesh->UpdateMesh();
        SplineMesh->bSmoothInterpRollScale =
            bSmoothInterpRollScale;
        OutSplineMeshes.Add(SplineMesh);
        
        
    }
    
}

void APBChoiceRouteActor::ClearSplineMeshes(TArray<TObjectPtr<USplineMeshComponent>>& SplineMeshes)
{

    for (USplineMeshComponent* SplineMesh : SplineMeshes)
    {
        if (!IsValid(SplineMesh))
        {
            continue;
        }

        RemoveInstanceComponent(SplineMesh);
        SplineMesh->DestroyComponent();
    }

    SplineMeshes.Empty();
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
    case EPBChoiceNodeType::Enhance:
    case EPBChoiceNodeType::Bet:
    case EPBChoiceNodeType::Battle:
    case EPBChoiceNodeType::Event:
    case EPBChoiceNodeType::Boss:
        FadeToNodeDestination(NodeType);
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


void APBChoiceRouteActor::FadeToNodeDestination(EPBChoiceNodeType NodeType)
{
    const FPBChoiceNodeDestination* Destination =
       NodeDestinations.Find(NodeType);

    if (!Destination)
    {
        UE_LOG(LogTemp, Warning, TEXT("No destination for NodeType=%d"), static_cast<int32>(NodeType));
        MoveToNextPoint();
        return;
    }

    PendingNodeType = NodeType;
    PendingCameraActor = Destination->CameraActor;
    PendingActionActor = Destination->ActionActor;

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
        FadeToDestinationTimerHandle,
        this,
        &APBChoiceRouteActor::OnFadeToDestinationFinished,
        FadeOutTime,
        false);
}

void APBChoiceRouteActor::OnFadeToDestinationFinished()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC || !PC->PlayerCameraManager)
    {
        return;
    }

    if (PendingCameraActor)
    {
        PC->SetViewTarget(PendingCameraActor);
    }

    if (PendingActionActor)
    {
        if (IPBChoiceNodeAction* Action =
            Cast<IPBChoiceNodeAction>(PendingActionActor))
        {
            Action->OpenAbility();
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("PendingActionActor does not implement IPBChoiceNodeAction: %s"),
                *GetNameSafe(PendingActionActor));
        }
    }

    PC->PlayerCameraManager->StartCameraFade(
        1.f,
        0.f,
        FadeInTime,
        FLinearColor::Black,
        false,
        false);
}

void APBChoiceRouteActor::HandleExitStart(FGameplayTag Exit, const FPBChoiceType& Message)
{
    FadeBackToBallAndMove();
}

void APBChoiceRouteActor::FadeBackToBallAndMove()
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
        FadeBackTimerHandle,
        this,
        &APBChoiceRouteActor::OnFadeBackToBallFinished,
        FadeOutTime,
        false);
}

void APBChoiceRouteActor::OnFadeBackToBallFinished()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

    if (!PC || !PC->PlayerCameraManager)
    {
        return;
    }

    if (ChoiceBallActor)
    {
        PC->SetViewTarget(ChoiceBallActor);
    }

    PC->PlayerCameraManager->StartCameraFade(
        1.f,
        0.f,
        FadeInTime,
        FLinearColor::Black,
        false,
        false);

    MoveToNextPoint();
}
