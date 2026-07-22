#include "PBTurtleFallingRock.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Subsystem/PBSoundSubsystem.h"
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

void APBTurtleFallingRock::SetSourcePatternName(const FName NewSourcePatternName)
{
	SourcePatternName = NewSourcePatternName;
}

void APBTurtleFallingRock::SetHitSFX(USoundBase* NewHitSFX)
{
	HitSFX = NewHitSFX;
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
			UE_LOG(LogTemp, Log, TEXT("[BossPatternDamage] Pattern=%s Damage=%d Target=%s"),
				*SourcePatternName.ToString(),
				DamageAmount,
				*GetNameSafe(Ball));
		}

		if (BounceVelocity > 0.0f)
		{
			if (IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(Ball))
			{
				FVector BounceDirection = Hit.ImpactNormal;
				BounceDirection.Z = 0.0f;
				if (!BounceDirection.Normalize())
				{
					BounceDirection = Ball->GetActorLocation() - GetActorLocation();
					BounceDirection.Z = 0.0f;
					BounceDirection.Normalize();
				}

				Movable->AddVelocity(BounceDirection * BounceVelocity);
			}
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

		if (UPBSoundSubsystem* SoundSubsystem = UPBSoundSubsystem::Get(this))
		{
			SoundSubsystem->PlaySFX(HitSFX);
		}
	}

	Destroy();
}
