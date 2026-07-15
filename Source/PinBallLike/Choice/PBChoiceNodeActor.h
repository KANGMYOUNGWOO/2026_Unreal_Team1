#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBChoiceNodeTypes.h"
#include "PBChoiceNodeActor.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;
class UTexture2D;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	TObjectPtr<UWidgetComponent> LabelWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	TObjectPtr<UWidgetComponent> PinWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	TObjectPtr<UWidgetComponent> FloorWidget;;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	FText LabelName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	UTexture2D* FloorImage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	UTexture2D* PinImage;
	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(EditAnywhere)
	EPBChoiceNodeType NodeType = EPBChoiceNodeType::None;
	
	UPROPERTY()
	int32 PointIndex = INDEX_NONE;
};