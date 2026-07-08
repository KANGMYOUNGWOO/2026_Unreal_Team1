#include "PBChoiceNodeManager.h"
#include "PBChoiceNodeActor.h"
#include "Components/SplineComponent.h"

APBChoiceNodeManager::APBChoiceNodeManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APBChoiceNodeManager::BuildNodes(
    USplineComponent* LeftSpline,
    USplineComponent* RightSpline)
{
    ClearNodes();

    BuildNodesFromSpline(EPBChoiceRouteSide::Left, LeftSpline);
    BuildNodesFromSpline(EPBChoiceRouteSide::Right, RightSpline);
}

void APBChoiceNodeManager::BuildNodesFromSpline(
    EPBChoiceRouteSide RouteSide,
    USplineComponent* TargetSpline)
{
    if (!TargetSpline)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildNodesFromSpline failed: TargetSpline is null."));
        return;
    }

    const int32 PointCount = TargetSpline->GetNumberOfSplinePoints();

    
    for (int32 PointIndex = 1; PointIndex < PointCount; ++PointIndex)
    {
        const EPBChoiceNodeType NodeType =
            DecideNodeType(RouteSide, PointIndex);

        if (NodeType == EPBChoiceNodeType::None)
        {
            continue;
        }

        TSubclassOf<APBChoiceNodeActor> NodeClass =
            GetNodeClassByType(NodeType);

        if (!NodeClass)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("No NodeClass for NodeType. Route=%d Point=%d"),
                static_cast<int32>(RouteSide),
                PointIndex);

            continue;
        }

        const FVector PointLocation =
            TargetSpline->GetLocationAtSplinePoint(
                PointIndex,
                ESplineCoordinateSpace::World);

        const FRotator PointRotation =
            TargetSpline->GetRotationAtSplinePoint(
                PointIndex,
                ESplineCoordinateSpace::World);

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;

        APBChoiceNodeActor* SpawnedNode =
            GetWorld()->SpawnActor<APBChoiceNodeActor>(
                NodeClass,
                PointLocation + NodeLocationOffset,
                FRotator::ZeroRotator,
                SpawnParams);

        if (!SpawnedNode)
        {
            continue;
        }

        SpawnedNode->SetupNode(NodeType, PointIndex);

        TMap<int32, TObjectPtr<APBChoiceNodeActor>>& TargetMap =
            GetNodeMapMutable(RouteSide);

        TargetMap.Add(PointIndex, SpawnedNode);
        
        const FVector SpawnLocation = PointLocation + NodeLocationOffset;

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[%s] Point %d  PointLocation=%s  SpawnLocation=%s"),
            RouteSide == EPBChoiceRouteSide::Left ? TEXT("Left") : TEXT("Right"),
            PointIndex,
            *PointLocation.ToString(),
            *SpawnLocation.ToString());
    }
}

APBChoiceNodeActor* APBChoiceNodeManager::GetNode(
    EPBChoiceRouteSide RouteSide,
    int32 PointIndex) const
{
    const TMap<int32, TObjectPtr<APBChoiceNodeActor>>& TargetMap =
        GetNodeMapConst(RouteSide);

    if (const TObjectPtr<APBChoiceNodeActor>* Found = TargetMap.Find(PointIndex))
    {
        return Found->Get();
    }

    return nullptr;
}

EPBChoiceNodeType APBChoiceNodeManager::HandleBallArrivedAtPoint(
    EPBChoiceRouteSide RouteSide,
    int32 PointIndex)
{
    APBChoiceNodeActor* Node = GetNode(RouteSide, PointIndex);

    if (!Node)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("No node found. Route=%d Point=%d"),
            static_cast<int32>(RouteSide),
            PointIndex);

        return EPBChoiceNodeType::None;
    }

    Node->PlayArriveEffect();

    const EPBChoiceNodeType NodeType = Node->GetNodeType();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Arrived Node. Route=%d Point=%d Type=%d"),
        static_cast<int32>(RouteSide),
        PointIndex,
        static_cast<int32>(NodeType));

    return NodeType;
}

void APBChoiceNodeManager::ClearNodes()
{
    for (TPair<int32, TObjectPtr<APBChoiceNodeActor>>& Pair : LeftNodes)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->Destroy();
        }
    }

    for (TPair<int32, TObjectPtr<APBChoiceNodeActor>>& Pair : RightNodes)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->Destroy();
        }
    }

    LeftNodes.Empty();
    RightNodes.Empty();
}

EPBChoiceNodeType APBChoiceNodeManager::DecideNodeType(
    EPBChoiceRouteSide RouteSide,
    int32 PointIndex) const
{
    const TMap<int32, EPBChoiceNodeType>* TargetMap = nullptr;

    switch (RouteSide)
    {
    case EPBChoiceRouteSide::Left:
        TargetMap = &LeftNodeTypes;
        break;

    case EPBChoiceRouteSide::Right:
        TargetMap = &RightNodeTypes;
        break;

    default:
        return EPBChoiceNodeType::None;
    }

    if (const EPBChoiceNodeType* FoundType = TargetMap->Find(PointIndex))
    {
        return *FoundType;
    }

    // 블루프린트에 해당 PointIndex가 등록되어 있지 않음
    UE_LOG(LogTemp, Warning,
      TEXT("NodeType not found. Route=%d PointIndex=%d"),
      static_cast<uint8>(RouteSide),
      PointIndex);
    
    return EPBChoiceNodeType::None;
}

TSubclassOf<APBChoiceNodeActor> APBChoiceNodeManager::GetNodeClassByType(
    EPBChoiceNodeType NodeType) const
{
    if (const TSubclassOf<APBChoiceNodeActor>* Found = NodeClassMap.Find(NodeType))
    {
        return *Found;
    }

    return nullptr;
}

TMap<int32, TObjectPtr<APBChoiceNodeActor>>& APBChoiceNodeManager::GetNodeMapMutable(
    EPBChoiceRouteSide RouteSide)
{
    return RouteSide == EPBChoiceRouteSide::Left
        ? LeftNodes
        : RightNodes;
}

const TMap<int32, TObjectPtr<APBChoiceNodeActor>>& APBChoiceNodeManager::GetNodeMapConst(
    EPBChoiceRouteSide RouteSide) const
{
    return RouteSide == EPBChoiceRouteSide::Left
        ? LeftNodes
        : RightNodes;
}