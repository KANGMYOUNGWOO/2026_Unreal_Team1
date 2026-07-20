#include "PBOctopusBoss.h"

#include "Animation/AnimSequence.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "PBOctopusTentacle.h"
#include "PinBallLike/Actor/Boss/Component/PBBossGroggyComponent.h"
#include "UObject/ConstructorHelpers.h"

APBOctopusBoss::APBOctopusBoss()
{
	TentacleClass = APBOctopusTentacle::StaticClass();

	static ConstructorHelpers::FObjectFinder<UAnimSequence> IdleAnimationFinder(
		TEXT("/Game/Blueprints/Boss/BossAsset/pulbo-monstruo/source/Boss8Armature_idle.Boss8Armature_idle"));
	if (IdleAnimationFinder.Succeeded())
	{
		IdleAnimation = IdleAnimationFinder.Object;
	}

	BodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(CollisionSphere);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	constexpr int32 TentacleSpawnPointCount = 4;
	TentacleSpawnPoints.Reserve(TentacleSpawnPointCount);

	for (int32 TentacleIndex = 0; TentacleIndex < TentacleSpawnPointCount; ++TentacleIndex)
	{
		const FName SpawnPointName(*FString::Printf(TEXT("TentacleSpawnPoint%d"), TentacleIndex + 1));
		USceneComponent* SpawnPoint = CreateDefaultSubobject<USceneComponent>(SpawnPointName);
		SpawnPoint->SetupAttachment(CollisionSphere);
		TentacleSpawnPoints.Add(SpawnPoint);
	}
}

void APBOctopusBoss::BeginPlay()
{
	Super::BeginPlay();
	PlayOctopusAnimation(IdleAnimation, true);
	SpawnTentacles();
}

void APBOctopusBoss::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyTentacles();
	Super::EndPlay(EndPlayReason);
}

void APBOctopusBoss::SpawnTentacles()
{
	DestroyTentacles();
	Tentacles.Reserve(TentacleSpawnPoints.Num());

	for (USceneComponent* SpawnPoint : TentacleSpawnPoints)
	{
		Tentacles.Add(SpawnTentacle(SpawnPoint));
	}

	OnTentaclesChanged.Broadcast();
}

APBOctopusTentacle* APBOctopusBoss::GetTentacle(int32 TentacleIndex) const
{
	return Tentacles.IsValidIndex(TentacleIndex) ? Tentacles[TentacleIndex] : nullptr;
}

void APBOctopusBoss::PlayOctopusAnimation(UAnimationAsset* Animation, const bool IsLooping)
{
	if (BodyMesh && Animation)
	{
		BodyMesh->PlayAnimation(Animation, IsLooping);
	}
}

void APBOctopusBoss::RestoreOctopusAnimationMode()
{
	if (BodyMesh)
	{
		BodyMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	}
}

void APBOctopusBoss::StartIdleState_Implementation()
{
	Super::StartIdleState_Implementation();
	PlayOctopusAnimation(IdleAnimation, true);
}

void APBOctopusBoss::HandleTentacleDestroyed(APBOctopusTentacle* DestroyedTentacle, int32 GroggyAmount)
{
	if (!IsValid(DestroyedTentacle) || !Tentacles.Contains(DestroyedTentacle))
	{
		return;
	}

	if (BossGroggyComponent && GroggyAmount > 0)
	{
		BossGroggyComponent->ApplyGroggyDamage(GroggyAmount);
	}
}

APBOctopusTentacle* APBOctopusBoss::SpawnTentacle(USceneComponent* SpawnPoint)
{
	UWorld* World = GetWorld();
	if (!World || !TentacleClass || !IsValid(SpawnPoint))
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;

	APBOctopusTentacle* Tentacle = World->SpawnActor<APBOctopusTentacle>(
		TentacleClass,
		SpawnPoint->GetComponentTransform(),
		SpawnParameters);
	if (Tentacle)
	{
		Tentacle->InitializeTentacle(this);
	}

	return Tentacle;
}

void APBOctopusBoss::DestroyTentacles()
{
	for (APBOctopusTentacle* Tentacle : Tentacles)
	{
		if (IsValid(Tentacle))
		{
			Tentacle->Destroy();
		}
	}

	Tentacles.Reset();
}
