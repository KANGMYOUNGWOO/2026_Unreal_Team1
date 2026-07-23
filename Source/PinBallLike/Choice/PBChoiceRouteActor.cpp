#include "PBChoiceRouteActor.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/GameViewportClient.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBorder.h"

#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PBChoiceBallActor.h"
#include "PBChoiceNodeManager.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Shop/MVVM/PBShopActor.h"
#include "UI/PBChoiceWidget.h"

namespace
{
    // Keep route fades above normal screens and below the persistent toolbar.
    constexpr int32 ChoiceScreenFadeZOrder = 9999;
}

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

void APBChoiceRouteActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    RemoveChoiceScreenFadeOverlay();
    Super::EndPlay(EndPlayReason);
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

    UpdateChoiceScreenFade(DeltaTime);

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

    StartChoiceScreenFade(
        0.0f,
        1.0f,
        FadeOutTime,
        FSimpleDelegate::CreateUObject(this, &APBChoiceRouteActor::OnFadeToDestinationFinished));
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

    StartChoiceScreenFade(1.0f, 0.0f, FadeInTime, FSimpleDelegate());
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

    StartChoiceScreenFade(
        0.0f,
        1.0f,
        FadeOutTime,
        FSimpleDelegate::CreateUObject(this, &APBChoiceRouteActor::OnFadeBackToBallFinished));
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

    StartChoiceScreenFade(1.0f, 0.0f, FadeInTime, FSimpleDelegate());

    MoveToNextPoint();
}

void APBChoiceRouteActor::StartChoiceScreenFade(
    const float FromOpacity,
    const float ToOpacity,
    const float Duration,
    FSimpleDelegate CompletionDelegate)
{
    EnsureChoiceScreenFadeOverlay();

    ChoiceScreenFadeFromOpacity = FMath::Clamp(FromOpacity, 0.0f, 1.0f);
    ChoiceScreenFadeToOpacity = FMath::Clamp(ToOpacity, 0.0f, 1.0f);
    ChoiceScreenFadeDuration = FMath::Max(0.0f, Duration);
    ChoiceScreenFadeElapsedTime = 0.0f;
    ChoiceScreenFadeCompletionDelegate = MoveTemp(CompletionDelegate);
    bChoiceScreenFadeActive = true;

    SetChoiceScreenFadeOpacity(ChoiceScreenFadeFromOpacity);

    if (ChoiceScreenFadeDuration <= 0.0f)
    {
        UpdateChoiceScreenFade(0.0f);
    }
}

void APBChoiceRouteActor::UpdateChoiceScreenFade(const float DeltaTime)
{
    if (!bChoiceScreenFadeActive)
    {
        return;
    }

    ChoiceScreenFadeElapsedTime += FMath::Max(0.0f, DeltaTime);
    const float Alpha = ChoiceScreenFadeDuration > 0.0f
        ? FMath::Clamp(ChoiceScreenFadeElapsedTime / ChoiceScreenFadeDuration, 0.0f, 1.0f)
        : 1.0f;

    SetChoiceScreenFadeOpacity(FMath::Lerp(ChoiceScreenFadeFromOpacity, ChoiceScreenFadeToOpacity, Alpha));

    if (Alpha < 1.0f)
    {
        return;
    }

    bChoiceScreenFadeActive = false;

    FSimpleDelegate CompletionDelegate = MoveTemp(ChoiceScreenFadeCompletionDelegate);
    ChoiceScreenFadeCompletionDelegate.Unbind();

    if (ChoiceScreenFadeToOpacity <= 0.0f)
    {
        RemoveChoiceScreenFadeOverlay();
    }

    CompletionDelegate.ExecuteIfBound();
}

void APBChoiceRouteActor::EnsureChoiceScreenFadeOverlay()
{
    if (ChoiceScreenFadeOverlay.IsValid())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!IsValid(World) || !World->GetGameViewport())
    {
        return;
    }

    ChoiceScreenFadeOverlay =
        SNew(SBorder)
        .BorderBackgroundColor(FLinearColor::Black)
        .Visibility(EVisibility::HitTestInvisible);

    World->GetGameViewport()->AddViewportWidgetContent(
        ChoiceScreenFadeOverlay.ToSharedRef(),
        ChoiceScreenFadeZOrder);
}

void APBChoiceRouteActor::RemoveChoiceScreenFadeOverlay()
{
    bChoiceScreenFadeActive = false;
    ChoiceScreenFadeCompletionDelegate.Unbind();

    if (!ChoiceScreenFadeOverlay.IsValid())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        if (UGameViewportClient* GameViewport = World->GetGameViewport())
        {
            GameViewport->RemoveViewportWidgetContent(ChoiceScreenFadeOverlay.ToSharedRef());
        }
    }

    ChoiceScreenFadeOverlay.Reset();
}

void APBChoiceRouteActor::SetChoiceScreenFadeOpacity(const float Opacity) const
{
    if (ChoiceScreenFadeOverlay.IsValid())
    {
        ChoiceScreenFadeOverlay->SetRenderOpacity(FMath::Clamp(Opacity, 0.0f, 1.0f));
    }
}
