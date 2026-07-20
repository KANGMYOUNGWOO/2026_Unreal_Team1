#include "PBOctopusTentacle.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PBOctopusBoss.h"
#include "PinBallLike/Actor/Boss/Component/PBBossHitEffectComponent.h"
#include "TimerManager.h"

APBOctopusTentacle::APBOctopusTentacle()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TentacleMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TentacleMesh"));
	TentacleMesh->SetupAttachment(SceneRoot);
	TentacleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TentacleMesh->SetCollisionResponseToAllChannels(ECR_Block);
	TentacleMesh->SetGenerateOverlapEvents(true);
	TentacleMesh->SetNotifyRigidBodyCollision(true);

	TelegraphStartPoint = CreateDefaultSubobject<USceneComponent>(TEXT("TelegraphStartPoint"));
	TelegraphStartPoint->SetupAttachment(TentacleMesh);

	HitEffectComponent = CreateDefaultSubobject<UPBBossHitEffectComponent>(TEXT("HitEffectComponent"));
}

void APBOctopusTentacle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	static_cast<void>(DeltaSeconds);

	const UWorld* World = GetWorld();
	if (!IsSlamActiveValue || !World || SlamDurationSeconds <= 0.0f)
	{
		return;
	}

	SlamProgress = FMath::Clamp(
		(World->GetTimeSeconds() - SlamStartTime) / SlamDurationSeconds,
		0.0f,
		1.0f);
}

void APBOctopusTentacle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RegenerationTimerHandle);
	Super::EndPlay(EndPlayReason);
}

bool APBOctopusTentacle::DamageToBoss_Implementation(int32 DamageAmount)
{
	const int32 PreviousTentacleHP = CurrentTentacleHP;
	ApplyTentacleDamage(DamageAmount);
	return CurrentTentacleHP < PreviousTentacleHP;
}

void APBOctopusTentacle::InitializeTentacle(APBOctopusBoss* NewOwnerBoss)
{
	OwnerBoss = NewOwnerBoss;
	DefaultActorLocation = GetActorLocation();
	DefaultActorRotation = GetActorRotation();
	IsDefaultActorRotationInitialized = true;
	MaxTentacleHP = FMath::Max(1, MaxTentacleHP);
	CurrentTentacleHP = MaxTentacleHP;
	IsTentacleDestroyed = false;
	SetTentacleActive(true);
	OnTentacleHPChanged.Broadcast(CurrentTentacleHP, MaxTentacleHP);
}

void APBOctopusTentacle::ApplyTentacleDamage(int32 DamageAmount)
{
	if (!IsTentacleAvailable() || IsSlamActiveValue || DamageAmount <= 0)
	{
		return;
	}

	CurrentTentacleHP = FMath::Max(0, CurrentTentacleHP - DamageAmount);
	if (HitEffectComponent)
	{
		HitEffectComponent->PlayHitEffect();
	}

	OnTentacleHPChanged.Broadcast(CurrentTentacleHP, MaxTentacleHP);
	if (CurrentTentacleHP <= 0)
	{
		DestroyTentacle();
	}
}

bool APBOctopusTentacle::IsTentacleAvailable() const
{
	return !IsTentacleDestroyed;
}

int32 APBOctopusTentacle::GetCurrentTentacleHP() const
{
	return CurrentTentacleHP;
}

int32 APBOctopusTentacle::GetMaxTentacleHP() const
{
	return MaxTentacleHP;
}

void APBOctopusTentacle::StartSlam(FVector NewSlamDirection, float SlamDuration)
{
	if (!IsTentacleAvailable() || SlamDuration <= 0.0f || !NewSlamDirection.Normalize())
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	SlamDirection = NewSlamDirection;
	const FVector SlamOriginBeforeRotation = TelegraphStartPoint
		? TelegraphStartPoint->GetComponentLocation()
		: GetActorLocation();
	FRotator SlamRotation = GetActorRotation();
	SlamRotation.Yaw = SlamDirection.Rotation().Yaw;
	SetActorRotation(SlamRotation);
	if (TelegraphStartPoint)
	{
		AddActorWorldOffset(SlamOriginBeforeRotation - TelegraphStartPoint->GetComponentLocation());
	}
	SlamDurationSeconds = SlamDuration;
	SlamStartTime = World->GetTimeSeconds();
	SlamProgress = 0.0f;
	IsSlamActiveValue = true;
	SetActorTickEnabled(true);
	BP_OnSlamStarted(SlamDirection, SlamDurationSeconds);
}

void APBOctopusTentacle::FinishSlam()
{
	if (!IsSlamActiveValue)
	{
		return;
	}

	IsSlamActiveValue = false;
	SlamProgress = 0.0f;
	SlamStartTime = 0.0f;
	SlamDurationSeconds = 0.0f;
	SetActorTickEnabled(false);
	if (IsDefaultActorRotationInitialized)
	{
		SetActorLocationAndRotation(DefaultActorLocation, DefaultActorRotation);
	}
	BP_OnSlamFinished();
}

bool APBOctopusTentacle::IsSlamActive() const
{
	return IsSlamActiveValue;
}

FVector APBOctopusTentacle::GetSlamDirection() const
{
	return SlamDirection;
}

float APBOctopusTentacle::GetSlamProgress() const
{
	return SlamProgress;
}

float APBOctopusTentacle::GetSlamControlRigWeight() const
{
	const float NormalizedPhase = SlamProgress <= 0.5f
		? SlamProgress * 2.0f
		: (1.0f - SlamProgress) * 2.0f;

	return FMath::InterpEaseInOut(0.0f, MaxSlamControlRigWeight, NormalizedPhase, 2.0f);
}

USkeletalMeshComponent* APBOctopusTentacle::GetTentacleMesh() const
{
	return TentacleMesh;
}

USceneComponent* APBOctopusTentacle::GetTelegraphStartComponent() const
{
	return TelegraphStartPoint;
}

void APBOctopusTentacle::DestroyTentacle()
{
	if (IsTentacleDestroyed)
	{
		return;
	}

	IsTentacleDestroyed = true;
	FinishSlam();
	SetTentacleActive(false);
	BP_OnTentacleDestroyed();
	OnTentacleDestroyed.Broadcast();

	if (OwnerBoss)
	{
		OwnerBoss->HandleTentacleDestroyed(this, DestroyedGroggyAmount);
	}

	if (RegenerationDelay <= 0.0f)
	{
		RegenerateTentacle();
		return;
	}

	GetWorldTimerManager().SetTimer(
		RegenerationTimerHandle,
		this,
		&APBOctopusTentacle::RegenerateTentacle,
		RegenerationDelay,
		false);
}

void APBOctopusTentacle::RegenerateTentacle()
{
	GetWorldTimerManager().ClearTimer(RegenerationTimerHandle);
	CurrentTentacleHP = MaxTentacleHP;
	IsTentacleDestroyed = false;
	SetTentacleActive(true);
	OnTentacleHPChanged.Broadcast(CurrentTentacleHP, MaxTentacleHP);
	BP_OnTentacleRegenerated();
	OnTentacleRegenerated.Broadcast();
}

void APBOctopusTentacle::SetTentacleActive(bool IsActive)
{
	if (!TentacleMesh)
	{
		return;
	}

	TentacleMesh->SetVisibility(IsActive, true);
	if (!IsActive)
	{
		TentacleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return;
	}

	TentacleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TentacleMesh->SetCollisionObjectType(ECC_WorldStatic);
	TentacleMesh->SetCollisionResponseToAllChannels(ECR_Block);
	TentacleMesh->SetGenerateOverlapEvents(true);
	TentacleMesh->SetNotifyRigidBodyCollision(true);
	TentacleMesh->SetAllBodiesCollisionObjectType(ECC_WorldStatic);
	TentacleMesh->SetAllBodiesNotifyRigidBodyCollision(true);
	TentacleMesh->RecreatePhysicsState();
}
