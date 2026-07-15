#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBChoiceNodeTypes.h"
#include "PBChoiceNodeManager.generated.h"

class USplineComponent;
class APBChoiceNodeActor;

UCLASS()
class PINBALLLIKE_API APBChoiceNodeManager : public AActor
{
	GENERATED_BODY()

public:
	APBChoiceNodeManager();

	void BuildNodes(
		USplineComponent* LeftSpline,
		USplineComponent* RightSpline);

	APBChoiceNodeActor* GetNode(
		EPBChoiceRouteSide RouteSide,
		int32 PointIndex) const;

	EPBChoiceNodeType HandleBallArrivedAtPoint(
		EPBChoiceRouteSide RouteSide,
		int32 PointIndex);

	void ClearNodes();

private:
	void GenerateRandomNodeTypes(
		USplineComponent* LeftSpline,
		USplineComponent* RightSpline);

	void GenerateRandomNodeTypesForRoute(
		USplineComponent* TargetSpline,
		TMap<int32, EPBChoiceNodeType>& OutNodeTypes);

	EPBChoiceNodeType GetRandomNormalNodeType() const;

private:
	UPROPERTY(EditAnywhere, Category = "Choice Node|Random")
	bool bUseRandomNodeTypes = true;
	
	
private:
	void BuildNodesFromSpline(
		EPBChoiceRouteSide RouteSide,
		USplineComponent* TargetSpline);

	EPBChoiceNodeType DecideNodeType(
	EPBChoiceRouteSide RouteSide,
	int32 PointIndex) const;

	TSubclassOf<APBChoiceNodeActor> GetNodeClassByType(
		EPBChoiceNodeType NodeType) const;

	TMap<int32, TObjectPtr<APBChoiceNodeActor>>& GetNodeMapMutable(
		EPBChoiceRouteSide RouteSide);

	const TMap<int32, TObjectPtr<APBChoiceNodeActor>>& GetNodeMapConst(
		EPBChoiceRouteSide RouteSide) const;

private:
	UPROPERTY(EditAnywhere, Category = "Choice Node")
	TMap<EPBChoiceNodeType, TSubclassOf<APBChoiceNodeActor>> NodeClassMap;

	UPROPERTY(EditAnywhere, Category = "Choice Node")
	FVector NodeLocationOffset = FVector(0.f, 0.f, 80.f);

	UPROPERTY()
	TMap<int32, TObjectPtr<APBChoiceNodeActor>> LeftNodes;

	UPROPERTY()
	TMap<int32, TObjectPtr<APBChoiceNodeActor>> RightNodes;
	
	UPROPERTY(EditAnywhere, Category = "Choice Node")
	TMap<int32, EPBChoiceNodeType> LeftNodeTypes;
	
	UPROPERTY(EditAnywhere, Category = "Choice Node")
	TMap<int32, EPBChoiceNodeType> RightNodeTypes;
	
	
};