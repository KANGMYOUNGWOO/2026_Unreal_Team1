#include "PBOctopusBindingZone.h"

#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

APBOctopusBindingZone::APBOctopusBindingZone()
{
	PrimaryActorTick.bCanEverTick = false;

	BindingArea = CreateDefaultSubobject<USphereComponent>(TEXT("BindingArea"));
	SetRootComponent(BindingArea);
	BindingArea->InitSphereRadius(100.0f);
	BindingArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BindingArea->SetCollisionObjectType(ECC_WorldDynamic);
	BindingArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	BindingArea->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	BindingArea->SetGenerateOverlapEvents(true);

	BindingEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BindingEffect"));
	BindingEffect->SetupAttachment(BindingArea);
	BindingEffect->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> BindingEffectFinder(
		TEXT("/Game/Blueprints/Boss/BossAsset/Niagara/Basic_VFX/Niagara/NS_Basic_1.NS_Basic_1"));
	if (BindingEffectFinder.Succeeded())
	{
		BindingEffect->SetAsset(BindingEffectFinder.Object);
	}
}

void APBOctopusBindingZone::InitializeZone(
	const float NewZoneRadius,
	const float NewZoneDuration,
	const float NewBindDuration,
	const int32 NewBindDamage,
	const float NewEffectScale)
{
	ZoneDuration = FMath::Max(NewZoneDuration, 0.0f);
	BindDuration = FMath::Max(NewBindDuration, 0.0f);
	BindDamage = FMath::Max(NewBindDamage, 0);
	const float EffectScale = FMath::Max(NewEffectScale, 0.0f);

	if (BindingArea)
	{
		BindingArea->SetSphereRadius(FMath::Max(NewZoneRadius, 0.0f));
	}

	if (BindingEffect)
	{
		BindingEffect->SetRelativeScale3D(FVector(EffectScale));
		BindingEffect->SetVariableFloat(TEXT("User.EmitterScale"), EffectScale);
	}
}

void APBOctopusBindingZone::BeginPlay()
{
	Super::BeginPlay();

	BindingArea->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&APBOctopusBindingZone::HandleZoneBeginOverlap);

	if (ZoneDuration <= 0.0f)
	{
		Destroy();
		return;
	}

	GetWorldTimerManager().SetTimer(
		ZoneDurationTimerHandle,
		this,
		&AActor::K2_DestroyActor,
		ZoneDuration,
		false);
}

void APBOctopusBindingZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(ZoneDurationTimerHandle);
	GetWorldTimerManager().ClearTimer(BindDurationTimerHandle);
	RestoreBoundBallMovement();

	Super::EndPlay(EndPlayReason);
}

void APBOctopusBindingZone::HandleZoneBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool IsFromSweep,
	const FHitResult& SweepResult)
{
	static_cast<void>(OverlappedComponent);
	static_cast<void>(OtherComponent);
	static_cast<void>(OtherBodyIndex);
	static_cast<void>(IsFromSweep);
	static_cast<void>(SweepResult);

	APBBallBase* Ball = Cast<APBBallBase>(OtherActor);
	if (!Ball || BoundBall)
	{
		return;
	}

	IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(Ball);
	if (!Movable)
	{
		return;
	}

	BoundBall = Ball;
	IsBoundBallPreviouslyPaused = Movable->IsMovementPaused();
	if (!IsBoundBallPreviouslyPaused)
	{
		Movable->PauseMovement();
	}

	if (BindDamage > 0)
	{
		if (IDamageable* Damageable = PBInterfaceUtils::FindInterface<IDamageable>(Ball))
		{
			if (!Damageable->IsDead())
			{
				Damageable->TakeDamage(BindDamage);
			}
		}
	}

	BindingArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetWorldTimerManager().ClearTimer(ZoneDurationTimerHandle);

	if (BindDuration <= 0.0f)
	{
		ReleaseBinding();
		return;
	}

	GetWorldTimerManager().SetTimer(
		BindDurationTimerHandle,
		this,
		&APBOctopusBindingZone::ReleaseBinding,
		BindDuration,
		false);
}

void APBOctopusBindingZone::ReleaseBinding()
{
	RestoreBoundBallMovement();
	Destroy();
}

void APBOctopusBindingZone::RestoreBoundBallMovement()
{
	if (IsValid(BoundBall) && !IsBoundBallPreviouslyPaused)
	{
		if (IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(BoundBall))
		{
			Movable->ResumeMovement();
		}
	}

	BoundBall = nullptr;
	IsBoundBallPreviouslyPaused = false;
}
