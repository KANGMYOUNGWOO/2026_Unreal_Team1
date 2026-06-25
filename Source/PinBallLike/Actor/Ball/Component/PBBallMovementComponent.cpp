<<<<<<<< HEAD:Source/PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.cpp
#include "PBBallPhysicsComponent.h"
========
#include "PBBallMovementComponent.h"
>>>>>>>> feature/ballstat:Source/PinBallLike/Actor/Ball/Component/PBBallMovementComponent.cpp

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBStatTypes.h"

<<<<<<<< HEAD:Source/PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.cpp
UPBBallPhysicsComponent::UPBBallPhysicsComponent()
========
UPBBallMovementComponent::UPBBallMovementComponent()
>>>>>>>> feature/ballstat:Source/PinBallLike/Actor/Ball/Component/PBBallMovementComponent.cpp
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;
}

<<<<<<<< HEAD:Source/PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.cpp
void UPBBallPhysicsComponent::InitializeDependencies(UPrimitiveComponent* InPrimitiveComponent, UPBBaseStatComponent* InStatComponent)
========
void UPBBallMovementComponent::BeginPlay()
>>>>>>>> feature/ballstat:Source/PinBallLike/Actor/Ball/Component/PBBallMovementComponent.cpp
{
	PrimitiveComponent = InPrimitiveComponent;
	StatComponent = InStatComponent;
}

<<<<<<<< HEAD:Source/PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.cpp
FVector UPBBallPhysicsComponent::GetVelocity() const
========
void UPBBallMovementComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
>>>>>>>> feature/ballstat:Source/PinBallLike/Actor/Ball/Component/PBBallMovementComponent.cpp
{
	return Velocity;
}

<<<<<<<< HEAD:Source/PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.cpp
void UPBBallPhysicsComponent::AddVelocity(FVector VelocityToAdd)
========
void UPBBallMovementComponent::SetVelocity(FVector NewVelocity)
{
	NewVelocity.Z = 0.0f;
	Velocity = NewVelocity;
}

void UPBBallMovementComponent::AddVelocity(FVector VelocityToAdd)
>>>>>>>> feature/ballstat:Source/PinBallLike/Actor/Ball/Component/PBBallMovementComponent.cpp
{
	VelocityToAdd.Z = 0.0f;
	Velocity += VelocityToAdd;
	Velocity.Z = 0.0f;
}

<<<<<<<< HEAD:Source/PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.cpp
void UPBBallPhysicsComponent::AddImpulse(FVector Impulse)
{
	Impulse.Z = 0.0f;
	const float SafeMass = FMath::Max(Mass, 0.001f);
	AddVelocity(Impulse / SafeMass);
}

void UPBBallPhysicsComponent::StopMovement()
{
	Velocity = FVector::ZeroVector;
}

void UPBBallPhysicsComponent::PauseMovement()
{
	bMovementPaused = true;
}

void UPBBallPhysicsComponent::ResumeMovement()
{
	bMovementPaused = false;
}

bool UPBBallPhysicsComponent::IsMovementPaused() const
{
	return bMovementPaused;
}

void UPBBallPhysicsComponent::SetMass(float NewMass)
{
	Mass = FMath::Max(NewMass, 0.001f);
}

float UPBBallPhysicsComponent::GetMass() const
{
	return Mass;
}

void UPBBallPhysicsComponent::SetBounceDamping(float NewBounceDamping)
{
	BounceDamping = FMath::Clamp(NewBounceDamping, 0.0f, 1.0f);
}

float UPBBallPhysicsComponent::GetBounceDamping() const
{
	return BounceDamping;
}

void UPBBallPhysicsComponent::SetVelocity(FVector NewVelocity)
{
	NewVelocity.Z = 0.0f;
	Velocity = NewVelocity;
}

void UPBBallPhysicsComponent::Launch(FVector Direction, const float Strength)
========
void UPBBallMovementComponent::Launch(FVector Direction, const float Strength)
>>>>>>>> feature/ballstat:Source/PinBallLike/Actor/Ball/Component/PBBallMovementComponent.cpp
{
	Direction.Z = 0.0f;
	const FVector NormalizedDirection = Direction.GetSafeNormal();
	Velocity = NormalizedDirection * FMath::Max(Strength, 0.0f);
}

<<<<<<<< HEAD:Source/PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.cpp
void UPBBallPhysicsComponent::MoveWithSweep(const float DeltaTime)
========
void UPBBallMovementComponent::Stop()
{
	Velocity = FVector::ZeroVector;
}

void UPBBallMovementComponent::MoveWithSweep(const float DeltaTime)
>>>>>>>> feature/ballstat:Source/PinBallLike/Actor/Ball/Component/PBBallMovementComponent.cpp
{
	// Tick에서 한 프레임 동안 이동할 전체 거리를 계산한다.
	// 이 함수는 위치를 직접 순간이동시키지 않고 Root Primitive를 Sweep 이동시킨다.
	if (!PrimitiveComponent || DeltaTime <= 0.0f)
	{
		return;
	}

	// RemainingMove는 이번 프레임에 아직 처리하지 못한 이동량이다.
	// 핀볼은 XY 평면만 사용하므로 Z 이동은 항상 제거한다.
	FVector RemainingMove = Velocity * DeltaTime;
	RemainingMove.Z = 0.0f;
	if (RemainingMove.IsNearlyZero())
	{
		return;
	}

	const int32 BounceLimit = FMath::Max(MaxBounceCountPerTick, 1);
	// 한 프레임 안에 여러 벽과 충돌할 수 있다. 충돌할 때마다 남은 이동량을
	// 반사 방향으로 다시 Sweep하되, 무한 반복을 막기 위해 횟수를 제한한다.
	for (int32 BounceCount = 0; BounceCount < BounceLimit; ++BounceCount)
	{
		FHitResult Hit;
		// MoveComponent는 충돌 지점까지만 실제 이동하고 Hit에 Sweep 결과를 기록한다.
		PrimitiveComponent->MoveComponent(
			RemainingMove,
			PrimitiveComponent->GetComponentQuat(),
			true,
			&Hit,
			MOVECOMP_NoFlags,
			ETeleportType::None);

		// Blocking Hit가 없으면 RemainingMove 전체를 이동한 것이므로 프레임 처리가 끝난다.
		if (!Hit.bBlockingHit)
		{
			break;
		}
		OnBallMovementHit.Broadcast(Hit);

		// 벽의 법선도 XY 평면으로 제한한 뒤 현재 진행 방향을 반사한다.
		FVector ImpactNormal = Hit.ImpactNormal;
		ImpactNormal.Z = 0.0f;
		ImpactNormal = ImpactNormal.GetSafeNormal();
		if (ImpactNormal.IsNearlyZero())
		{
			Velocity = FVector::ZeroVector;
			break;
		}

		const float PreviousSpeed = Velocity.Size2D();
		const FVector IncomingDirection = Velocity.GetSafeNormal2D();
		FVector ReflectedDirection = FMath::GetReflectionVector(IncomingDirection, ImpactNormal);
		ReflectedDirection.Z = 0.0f;
		ReflectedDirection = ReflectedDirection.GetSafeNormal();

		const float ImpactDamping = CalculateImpactDamping(IncomingDirection, ImpactNormal);
		Velocity = ReflectedDirection * PreviousSpeed * ImpactDamping;

		// Hit.Time은 요청한 이동량 중 충돌 전까지 실제로 이동한 비율이다.
		// 예: Hit.Time이 0.8이면 80%를 이동했고, 남은 20%만 반사 방향으로 처리한다.
		const float RemainingFraction = 1.0f - FMath::Clamp(Hit.Time, 0.0f, 1.0f);
		const float RemainingDistance = RemainingMove.Size2D() * RemainingFraction;
		RemainingMove = ReflectedDirection * RemainingDistance;

		if (RemainingMove.IsNearlyZero())
		{
			break;
		}
	}

	// 모든 Sweep 처리가 끝난 뒤 다음 Tick에 사용할 속도를 허용 범위로 보정한다.
	ClampVelocityToSpeedRange();
}

<<<<<<<< HEAD:Source/PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.cpp
void UPBBallPhysicsComponent::BeginPlay()
========
UPrimitiveComponent* UPBBallMovementComponent::ResolveCollisionComponent()
>>>>>>>> feature/ballstat:Source/PinBallLike/Actor/Ball/Component/PBBallMovementComponent.cpp
{
	Super::BeginPlay();

	if (!PrimitiveComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("%s requires a UPrimitiveComponent as its owner's root component."), *GetName());
		SetComponentTickEnabled(false);
	}
	
	if (StatComponent)
	{
		StatComponent->OnStatChanged.AddUObject(
			this,
			&UPBBallPhysicsComponent::HandleStatChanged);

		ApplyStat(PBStatNames::Mass, StatComponent->GetStat(PBStatNames::Mass));
		ApplyStat(PBStatNames::Bounciness, StatComponent->GetStat(PBStatNames::Bounciness));
	}
}

<<<<<<<< HEAD:Source/PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.cpp
void UPBBallPhysicsComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime <= 0.0f || bMovementPaused)
	{
		return;
	}

	// XY 평면 속도에 인공 중력을 적용한 뒤 이번 프레임 이동을 처리한다.
	Velocity.Z = 0.0f;
	Velocity.X -= XGravity * DeltaTime;
	MoveWithSweep(DeltaTime);
}

void UPBBallPhysicsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (StatComponent)
	{
		StatComponent->OnStatChanged.RemoveAll(this);
	}
	Super::EndPlay(EndPlayReason);
}

float UPBBallPhysicsComponent::CalculateImpactDamping(
========
float UPBBallMovementComponent::CalculateImpactDamping(
>>>>>>>> feature/ballstat:Source/PinBallLike/Actor/Ball/Component/PBBallMovementComponent.cpp
	const FVector IncomingDirection,
	const FVector ImpactNormal) const
{
	// 스치는 충돌은 0, 정면 충돌은 1에 가까운 값이 된다.
	const float ImpactStrength = FMath::Clamp(
		-FVector::DotProduct(IncomingDirection, ImpactNormal), 0.0f, 1.0f);

	return FMath::Lerp(1.0f, BounceDamping, ImpactStrength);
}

<<<<<<<< HEAD:Source/PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.cpp
void UPBBallPhysicsComponent::ClampVelocityToSpeedRange()
========
void UPBBallMovementComponent::ClampVelocityToSpeedRange()
>>>>>>>> feature/ballstat:Source/PinBallLike/Actor/Ball/Component/PBBallMovementComponent.cpp
{
	// 이동 방향은 유지하면서 속력만 MinSpeed와 MaxSpeed 범위로 제한한다.
	Velocity.Z = 0.0f;

	const float CurrentSpeed = Velocity.Size2D();
	if (FMath::IsNearlyZero(CurrentSpeed))
	{
		Velocity = FVector::ZeroVector;
		return;
	}

	const float SafeMinSpeed = FMath::Max(MinSpeed, 0.0f);
	const float SafeMaxSpeed = FMath::Max(MaxSpeed, SafeMinSpeed);
	Velocity = Velocity.GetSafeNormal2D() * FMath::Clamp(CurrentSpeed, SafeMinSpeed, SafeMaxSpeed);
}

void UPBBallPhysicsComponent::HandleStatChanged(FName StatName, int32 NewValue)
{
	ApplyStat(StatName, NewValue);
}

void UPBBallPhysicsComponent::ApplyStat(FName StatName, int32 NewValue)
{
	if (StatName == PBStatNames::Mass)
	{
		SetMass(static_cast<float>(NewValue));
	}
	else if (StatName == PBStatNames::Bounciness)
	{
		SetBounceDamping(static_cast<float>(NewValue) / 100.0f);
	}
}
