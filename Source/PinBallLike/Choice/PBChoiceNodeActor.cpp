#include "PBChoiceNodeActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"

APBChoiceNodeActor::APBChoiceNodeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	LabelWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("LabelWidget"));
	LabelWidget->SetupAttachment(Mesh);
	LabelWidget->SetWidgetSpace(EWidgetSpace::World);
	LabelWidget->SetDrawSize(FVector2D(300.f, 120.f));
	LabelWidget->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	LabelWidget->SetTwoSided(true);
}

void APBChoiceNodeActor::SetupNode(EPBChoiceNodeType InNodeType, int32 InPointIndex)
{
	NodeType = InNodeType;
	PointIndex = InPointIndex;
	
}

void APBChoiceNodeActor::PlayArriveEffect()
{
	UE_LOG(LogTemp, Warning, TEXT("Arrived Node Point: %d"), PointIndex);

	// 임시 연출
	SetActorScale3D(FVector(1.2f));
}