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
	LabelWidget->SetDrawSize(FVector2D(300.f, 300.f));
	LabelWidget->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	LabelWidget->SetTwoSided(true);
	
	PinWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PinWidget"));
	PinWidget->SetupAttachment(Mesh);
	PinWidget->SetWidgetSpace(EWidgetSpace::World);
	PinWidget->SetDrawSize(FVector2D(100.f, 100.f));
	PinWidget->SetRelativeLocation(FVector(-106,0,146));
	PinWidget->SetRelativeRotation(FRotator(120,180,180));
	PinWidget->SetTwoSided(true);
	
	FloorWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("FloorWidget"));
	FloorWidget->SetupAttachment(Mesh);
	FloorWidget->SetWidgetSpace(EWidgetSpace::World);
	FloorWidget->SetDrawSize(FVector2D(320.f, 300.f));
	FloorWidget->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	FloorWidget->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	FloorWidget->SetTwoSided(true);
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