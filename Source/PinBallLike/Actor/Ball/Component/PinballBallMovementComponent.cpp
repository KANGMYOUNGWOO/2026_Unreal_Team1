#include "PinballBallMovementComponent.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

UPinballBallMovementComponent::UPinballBallMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;
}

void UPinballBallMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// Owner의 Root Primitive를 실제 이동 및 충돌 컴포넌트로 사용한다.
	CollisionComponent = ResolveCollisionComponent();
	if (!CollisionComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("%s requires a UPrimitiveComponent as its owner's root component."),
			*GetName());
		SetComponentTickEnabled(false);
	}
}

void UPinballBallMovementComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime <= 0.0f)
	{
		return;
	}

	// XY 평면 속도에 인공 중력을 적용한 뒤 이번 프레임 이동을 처리한다.
	Velocity.Z = 0.0f;
	Velocity.X -= XGravity * DeltaTime;
	MoveWithSweep(DeltaTime);
}

void UPinballBallMovementComponent::SetVelocity(FVector NewVelocity)
{
	NewVelocity.Z = 0.0f;
	Velocity = NewVelocity;
}

void UPinballBallMovementComponent::AddVelocity(FVector VelocityToAdd)
{
	VelocityToAdd.Z = 0.0f;
	Velocity += VelocityToAdd;
	Velocity.Z = 0.0f;
}

void UPinballBallMovementComponent::Launch(FVector Direction, const float Strength)
{
	Direction.Z = 0.0f;
	const FVector NormalizedDirection = Direction.GetSafeNormal();
	Velocity = NormalizedDirection * FMath::Max(Strength, 0.0f);
}

void UPinballBallMovementComponent::Stop()
{
	Velocity = FVector::ZeroVector;
}

void UPinballBallMovementComponent::MoveWithSweep(const float DeltaTime)
{
	// Tick에서 한 프레임 동안 이동할 전체 거리를 계산한다.
	// 이 함수는 위치를 직접 순간이동시키지 않고 Root Primitive를 Sweep 이동시킨다.
	UPrimitiveComponent* RootPrimitive = ResolveCollisionComponent();
	if (!RootPrimitive || DeltaTime <= 0.0f)
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
		RootPrimitive->MoveComponent(
			RemainingMove,
			RootPrimitive->GetComponentQuat(),
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
			Stop();
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

UPrimitiveComponent* UPinballBallMovementComponent::ResolveCollisionComponent()
{
	// 캐시된 컴포넌트가 없으면 Owner의 Root에서 다시 찾는다.
	if (IsValid(CollisionComponent))
	{
		return CollisionComponent;
	}

	AActor* Owner = GetOwner();
	CollisionComponent = Owner ? Cast<UPrimitiveComponent>(Owner->GetRootComponent()) : nullptr;
	return CollisionComponent;
}

float UPinballBallMovementComponent::CalculateImpactDamping(
	const FVector IncomingDirection,
	const FVector ImpactNormal) const
{
	// 스치는 충돌은 0, 정면 충돌은 1에 가까운 값이 된다.
	const float ImpactStrength = FMath::Clamp(
		-FVector::DotProduct(IncomingDirection, ImpactNormal), 0.0f, 1.0f);

	return FMath::Lerp(1.0f, BounceDamping, ImpactStrength);
}

void UPinballBallMovementComponent::ClampVelocityToSpeedRange()
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
