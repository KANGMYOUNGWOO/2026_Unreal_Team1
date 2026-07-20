#include "PBBumperPickupActor.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "PinBallLike/Actor/Bumper/Reward/PBBumperRewardUtils.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

APBBumperPickupActor::APBBumperPickupActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CollectionArea = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionArea"));
	CollectionArea->SetupAttachment(SceneRoot);
	CollectionArea->SetSphereRadius(60.0f);
	CollectionArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollectionArea->SetCollisionObjectType(ECC_WorldDynamic);
	CollectionArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollectionArea->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	CollectionArea->SetGenerateOverlapEvents(true);
	CollectionArea->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&APBBumperPickupActor::HandleCollectionAreaBeginOverlap);

	PickupVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupVisual"));
	PickupVisual->SetupAttachment(CollectionArea);
	PickupVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupVisual->SetGenerateOverlapEvents(false);
	PickupVisual->SetRelativeScale3D(FVector(0.5f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		PickupVisual->SetStaticMesh(SphereMesh.Object);
	}

	PickupLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PickupLight"));
	PickupLight->SetupAttachment(CollectionArea);
	PickupLight->SetIntensity(1500.0f);
	PickupLight->SetAttenuationRadius(220.0f);
	PickupLight->SetCastShadows(false);

	SetPickupActive(false);
}

void APBBumperPickupActor::StartActionForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LifetimeTimerHandle);
	}

	SetPickupActive(true);
	Super::StartActionForActor(Bumper, InteractionActor);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			LifetimeTimerHandle,
			this,
			&APBBumperPickupActor::HandleLifetimeExpired,
			Lifetime,
			false);
	}
}

void APBBumperPickupActor::DeactivateSummon()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LifetimeTimerHandle);
	}

	SetPickupActive(false);
	Super::DeactivateSummon();
}

void APBBumperPickupActor::ConfigurePickup(
	const EPBBumperRewardType InRewardType,
	const FName InResourceName,
	const FName InStatusEffectId,
	const float InRewardPower,
	const float InLifetime,
	const FLinearColor& InRewardColor)
{
	RewardType = InRewardType;
	ResourceName = InResourceName;
	StatusEffectId = InStatusEffectId;
	RewardPower = FMath::IsFinite(InRewardPower) ? FMath::Max(InRewardPower, 0.0f) : 0.0f;
	Lifetime = FMath::IsFinite(InLifetime) ? FMath::Clamp(InLifetime, 0.5f, 60.0f) : 8.0f;
	PickupLight->SetLightColor(InRewardColor);
}

void APBBumperPickupActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LifetimeTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void APBBumperPickupActor::SetPickupActive(const bool bNewActive)
{
	bIsPickupActive = bNewActive;
	CollectionArea->SetCollisionEnabled(
		bNewActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	PickupVisual->SetVisibility(bNewActive, true);
	PickupLight->SetVisibility(bNewActive, true);
}

void APBBumperPickupActor::HandleLifetimeExpired()
{
	UE_LOG(LogTemp, Verbose,
		TEXT("[Bumper] Pickup expired without collection. Pickup=%s"),
		*GetNameSafe(this));
	FinishAction();
}

void APBBumperPickupActor::HandleCollectionAreaBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	static_cast<void>(OverlappedComponent);
	static_cast<void>(OtherComp);
	static_cast<void>(OtherBodyIndex);
	static_cast<void>(bFromSweep);
	static_cast<void>(SweepResult);

	if (!bIsPickupActive || !IsValid(OtherActor))
	{
		return;
	}

	const FPBBumperRewardApplyResult Result = PBBumperRewardUtils::ApplyReward(
		OtherActor,
		RewardType,
		ResourceName,
		StatusEffectId,
		RewardPower);
	if (!Result.bApplied)
	{
		return;
	}

	UE_LOG(LogTemp, Log,
		TEXT("[Bumper] Pickup collected. Pickup=%s Target=%s RewardType=%s AppliedValue=%.2f AppliedCount=%d"),
		*GetNameSafe(this),
		*GetNameSafe(OtherActor),
		*UEnum::GetValueAsString(RewardType),
		Result.AppliedValue,
		Result.AppliedCount);
	FinishAction();
}
