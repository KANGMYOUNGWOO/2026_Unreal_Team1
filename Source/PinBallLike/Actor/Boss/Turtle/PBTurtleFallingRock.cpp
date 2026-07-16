#include "PBTurtleFallingRock.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"
#include "UObject/ConstructorHelpers.h"

APBTurtleFallingRock::APBTurtleFallingRock()
{
	PrimaryActorTick.bCanEverTick = false;

	RockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RockMesh"));
	SetRootComponent(RockMesh);
	RockMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RockMesh->SetCollisionResponseToAllChannels(ECR_Block);
	RockMesh->SetNotifyRigidBodyCollision(true);

	FallingMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("FallingMovement"));
	FallingMovement->SetUpdatedComponent(RockMesh);
	FallingMovement->ProjectileGravityScale = 0.0f;
	FallingMovement->bRotationFollowsVelocity = false;
	FallingMovement->bShouldBounce = false;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> HitEffectFinder(
		TEXT("/Game/Blueprints/Boss/BossAsset/Niagara/Free_Magic/VFX_Niagara/NS_Free_Magic_Attack1.NS_Free_Magic_Attack1"));
	HitEffect = HitEffectFinder.Object;
}

void APBTurtleFallingRock::BeginPlay()
{
	Super::BeginPlay();

	if (RockMesh)
	{
		RockMesh->OnComponentHit.AddUniqueDynamic(this, &APBTurtleFallingRock::HandleRockHit);
	}
}

void APBTurtleFallingRock::SetFallingSpeed(float NewFallingSpeed)
{
	if (!FallingMovement)
	{
		return;
	}

	const float FallingSpeed = FMath::Max(NewFallingSpeed, 0.0f);
	FallingMovement->InitialSpeed = FallingSpeed;
	FallingMovement->MaxSpeed = FallingSpeed;
	FallingMovement->Velocity = FVector::DownVector * FallingSpeed;
}

void APBTurtleFallingRock::HandleRockHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (APBBallBase* Ball = Cast<APBBallBase>(OtherActor))
	{
		IDamageable* Damageable = Cast<IDamageable>(Ball);
		if (!Damageable)
		{
			Damageable = PBInterfaceUtils::FindInterface<IDamageable>(Ball);
		}

		if (Damageable && !Damageable->IsDead() && DamageAmount > 0)
		{
			Damageable->TakeDamage(DamageAmount);
		}
	}

	if (HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			HitEffect,
			Hit.ImpactPoint,
			FRotator::ZeroRotator,
			GetActorScale3D());
	}

	Destroy();
}
