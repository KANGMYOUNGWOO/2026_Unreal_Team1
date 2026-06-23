// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperReactionComponent.h"

#include "Components/SceneComponent.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/Actor.h"

UPBBumperReactionComponent::UPBBumperReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPBBumperReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	ResolveTargetComponent();
	CacheBaseTransform();
	InitializeReactionTimeline();
}

void UPBBumperReactionComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ReactionTimeline.TickTimeline(DeltaTime);
}

void UPBBumperReactionComponent::PlayImpactReaction(const FHitResult& Hit)
{
	if (!ResolveTargetComponent())
	{
		return;
	}

	//연출 중 새로운 요청이 들어오면 초기화 하고 다시 수행
	ReactionTimeline.Stop();
	ResetTargetTransform();

	CurrentPushDirection = Hit.ImpactNormal;
	CurrentPushDirection.Z = 0.0f;
	if (!CurrentPushDirection.Normalize())
	{
		CurrentPushDirection = FVector::ZeroVector;
	}

	ReactionTimeline.PlayFromStart();
	SetComponentTickEnabled(true);
}

void UPBBumperReactionComponent::InitializeReactionTimeline()
{
	//타임라인에 커브 연결
	if (MoveCurve)
	{
		FOnTimelineFloat MoveUpdateFunction;
		MoveUpdateFunction.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UPBBumperReactionComponent, HandleMoveTimelineUpdate));
		ReactionTimeline.AddInterpFloat(MoveCurve, MoveUpdateFunction);
	}

	if (ScaleCurve)
	{
		FOnTimelineFloat ScaleUpdateFunction;
		ScaleUpdateFunction.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UPBBumperReactionComponent, HandleScaleTimelineUpdate));
		ReactionTimeline.AddInterpFloat(ScaleCurve, ScaleUpdateFunction);
	}

	FOnTimelineEvent FinishedFunction;
	FinishedFunction.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UPBBumperReactionComponent, HandleReactionTimelineFinished));
	ReactionTimeline.SetTimelineFinishedFunc(FinishedFunction);
	ReactionTimeline.SetLooping(false);
}

// TargetComponent비었으면 BumperVisual 태그가 붙은 컴포넌트를 찾아 사용.
bool UPBBumperReactionComponent::ResolveTargetComponent()
{
	if (IsValid(TargetComponent))
	{
		return true;
	}

	AActor* Owner = GetOwner();
	if (!Owner || TargetComponentTag.IsNone())
	{
		return false;
	}

	TArray<UActorComponent*> TaggedComponents = Owner->GetComponentsByTag(
		USceneComponent::StaticClass(), TargetComponentTag);
	
	for (UActorComponent* TaggedComponent : TaggedComponents)
	{
		TargetComponent = Cast<USceneComponent>(TaggedComponent);
		if (IsValid(TargetComponent))
		{
			CacheBaseTransform();
			return true;
		}
	}

	return false;
}

void UPBBumperReactionComponent::CacheBaseTransform()
{
	if (!IsValid(TargetComponent))
	{
		return;
	}

	BaseRelativeLocation = TargetComponent->GetRelativeLocation();
	BaseRelativeScale = TargetComponent->GetRelativeScale3D();
}

void UPBBumperReactionComponent::ResetTargetTransform()
{
	if (IsValid(TargetComponent))
	{
		TargetComponent->SetRelativeLocation(BaseRelativeLocation);
		TargetComponent->SetRelativeScale3D(BaseRelativeScale);
	}
}

void UPBBumperReactionComponent::HandleMoveTimelineUpdate(const float Alpha)
{
	if (!IsValid(TargetComponent))
	{
		return;
	}
	const FVector TargetLocation = BaseRelativeLocation + CurrentPushDirection * PushDistance * Alpha;
	TargetComponent->SetRelativeLocation(TargetLocation);
}

void UPBBumperReactionComponent::HandleScaleTimelineUpdate(const float Alpha)
{
	if (!IsValid(TargetComponent))
	{
		return;
	}
	const FVector TargetScale = BaseRelativeScale * Alpha;
	TargetComponent->SetRelativeScale3D(TargetScale);
}

void UPBBumperReactionComponent::HandleReactionTimelineFinished()
{
	ResetTargetTransform();

	SetComponentTickEnabled(false);
}
