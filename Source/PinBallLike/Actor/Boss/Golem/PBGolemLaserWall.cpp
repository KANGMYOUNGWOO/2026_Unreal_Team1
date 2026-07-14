#include "PBGolemLaserWall.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

APBGolemLaserWall::APBGolemLaserWall()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(SceneRoot);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionBox->SetNotifyRigidBodyCollision(true);
	CollisionBox->OnComponentHit.AddUniqueDynamic(this, &APBGolemLaserWall::HandleCollisionHit);
}

void APBGolemLaserWall::InitializeLaserWall(
	FVector InLaunchDirection,
	float InBounceVelocity,
	FName InSourcePatternName)
{
	LaunchDirection = InLaunchDirection;
	LaunchDirection.Z = 0.0f;
	LaunchDirection = LaunchDirection.GetSafeNormal();

	if (LaunchDirection.IsNearlyZero())
	{
		LaunchDirection = GetActorForwardVector().GetSafeNormal2D();
	}

	BounceVelocity = FMath::Max(0.0f, InBounceVelocity);
	SourcePatternName = InSourcePatternName;
}

void APBGolemLaserWall::HandleCollisionHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	static_cast<void>(HitComponent);
	static_cast<void>(OtherComp);
	static_cast<void>(NormalImpulse);

	UE_LOG(LogTemp, Log, TEXT("[GolemLaserWall] Hit detected. LaserWall=%s OtherActor=%s ImpactPoint=%s ImpactNormal=%s"),
		*GetNameSafe(this),
		*GetNameSafe(OtherActor),
		*Hit.ImpactPoint.ToString(),
		*Hit.ImpactNormal.ToString());

	BounceBall(OtherActor, Hit);
}

void APBGolemLaserWall::BounceBall(AActor* OtherActor, const FHitResult& Hit)
{
	APBBallBase* Ball = Cast<APBBallBase>(OtherActor);
	if (!IsValid(Ball))
	{
		UE_LOG(LogTemp, Log, TEXT("[GolemLaserWall] Bounce skipped. Other actor is not a ball. OtherActor=%s"),
			*GetNameSafe(OtherActor));
		return;
	}

	IDamageable* Damageable = PBInterfaceUtils::FindInterface<IDamageable>(Ball);
	if (Damageable && !Damageable->IsDead())
	{
		constexpr int32 DamageAmount = 1;
		Damageable->TakeDamage(DamageAmount);
		UE_LOG(LogTemp, Log, TEXT("[BossPatternDamage] Pattern=%s Damage=%d Target=%s"),
			*SourcePatternName.ToString(),
			DamageAmount,
			*GetNameSafe(Ball));
	}

	IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(Ball);
	if (!Movable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GolemLaserWall] Bounce skipped. IMovable was not found. Ball=%s"),
			*GetNameSafe(Ball));
		return;
	}

	FVector BounceDirection = Hit.ImpactNormal;
	BounceDirection.Z = 0.0f;

	if (BounceDirection.IsNearlyZero())
	{
		BounceDirection = LaunchDirection;
	}

	BounceDirection = BounceDirection.GetSafeNormal();
	if (BounceDirection.IsNearlyZero())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GolemLaserWall] Bounce skipped. Bounce direction is invalid. Ball=%s"),
			*GetNameSafe(Ball));
		return;
	}

	const FVector AddedVelocity = BounceDirection * BounceVelocity;
	UE_LOG(LogTemp, Log, TEXT("[GolemLaserWall] AddVelocity. Ball=%s Direction=%s AddedVelocity=%s"),
		*GetNameSafe(Ball),
		*BounceDirection.ToString(),
		*AddedVelocity.ToString());

	Movable->AddVelocity(AddedVelocity);
}
