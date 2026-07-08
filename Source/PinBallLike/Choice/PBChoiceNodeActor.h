#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBChoiceNodeTypes.h"
#include "PBChoiceNodeActor.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;

UCLASS()
class PINBALLLIKE_API APBChoiceNodeActor : public AActor
{
	GENERATED_BODY()

public:
	APBChoiceNodeActor();

	void SetupNode(EPBChoiceNodeType InNodeType, int32 InPointIndex);
	void PlayArriveEffect();

	EPBChoiceNodeType GetNodeType() const { return NodeType; }
	int32 GetPointIndex() const { return PointIndex; }

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> LabelWidget;

	UPROPERTY(EditAnywhere)
	EPBChoiceNodeType NodeType = EPBChoiceNodeType::None;

	UPROPERTY()
	int32 PointIndex = INDEX_NONE;
};