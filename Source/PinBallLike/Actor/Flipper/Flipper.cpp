// Fill out your copyright notice in the Description page of Project Settings.


#include "Flipper.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#if ENABLE_DRAW_DEBUG
#include "DrawDebugHelpers.h"
#endif
#include "Math/UnrealMathUtility.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

AFlipper::AFlipper()
{
	PrimaryActorTick.bCanEverTick = true;

	Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	SetRootComponent(Pivot);

	BallDetectTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("BallDetectTrigger"));
	BallDetectTrigger->SetupAttachment(Pivot);
	BallDetectTrigger->InitBoxExtent(FVector(50.0f, 150.0f, 50.0f));
	BallDetectTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BallDetectTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	BallDetectTrigger->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	BallDetectTrigger->SetGenerateOverlapEvents(true);
}

void AFlipper::BeginPlay()
{
	Super::BeginPlay();

	DownedRotation = Pivot->GetRelativeRotation();
}

void AFlipper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	const bool bMoved = UpdateRotation(DeltaTime);
	if (bMoved && bIsMoveUp)
	{
		UpdateFlipperForce(DeltaTime);
	}

	DrawForceDirectionDebug();
}

bool AFlipper::UpdateRotation(const float DeltaTime)
{
	// 반환값은 이번 프레임에 실제 회전이 발생했는지 나타낸다.
	const FRotator CurrentRotation = Pivot->GetRelativeRotation();
	const FRotator TargetRotation = bIsMoveUp
		? DownedRotation + GetAppliedRotationOffset()
		: DownedRotation;

	const FRotator NewRotation = FMath::RInterpConstantTo(
		CurrentRotation,
		TargetRotation,
		DeltaTime,
		RotationSpeed);

	Pivot->SetRelativeRotation(NewRotation);

	const bool bMoved = !NewRotation.Equals(CurrentRotation, KINDA_SMALL_NUMBER);

	return bMoved;
}

void AFlipper::UpdateFlipperForce(const float DeltaTime)
{
	// 플리퍼가 움직인 정도에 따라 가하는 힘을 다르게 한다.
	// 움직임 시작 1, 움직임 종료 0
	const float MotionAlpha = CalculateMotionAlpha();
	if (MotionAlpha > 0.0f)
	{
		ApplyForceToBalls(DeltaTime, MotionAlpha);
	}
}

FRotator AFlipper::GetAppliedRotationOffset() const
{
	// 프로젝트 규칙은 양수 Yaw = 왼쪽, 음수 Yaw = 오른쪽이다.
	// Unreal의 실제 Pivot 회전 방향과 의미상의 좌우 방향을 맞추기 위해 적용 시 Yaw를 반전한다.
	FRotator AppliedOffset = MoveUpRotationOffset;
	AppliedOffset.Yaw *= -1.0f;
	return AppliedOffset;
}

float AFlipper::CalculateMotionAlpha() const
{
	// 반환값은 상승 시작 1.0, 상승 완료 0.0인 힘 배율이다.
	const FRotator MoveUpRotation = DownedRotation + GetAppliedRotationOffset();
	const float TotalMoveUpAngle = DownedRotation.Quaternion().AngularDistance(MoveUpRotation.Quaternion());
	if (TotalMoveUpAngle <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	const float RemainingMoveUpAngle =
		Pivot->GetRelativeRotation().Quaternion().AngularDistance(MoveUpRotation.Quaternion());
	const float progress = FMath::Clamp(1.0f - RemainingMoveUpAngle / TotalMoveUpAngle, 0.0f, 1.0f);
	return 1.0f - progress;
}

float AFlipper::GetFlipperLength() const
{
	// Ball 감지 Trigger의 스케일이 적용된 Y축 전체 길이를 사용한다.
	return FMath::Max(BallDetectTrigger->GetScaledBoxExtent().Y * 2.0f, 1.0f);
}

float AFlipper::CalculateDistanceAlpha(const FVector WorldLocation) const
{
	// Pivot은 0.0, Flipper 끝부분은 1.0이 되도록 거리를 정규화한다.
	const float DistanceFromPivot = FVector::Dist2D(
		Pivot->GetComponentLocation(), WorldLocation);
	return FMath::Clamp(DistanceFromPivot / GetFlipperLength(), 0.0f, 1.0f);
}

FVector AFlipper::CalculateForceDirection(const float DistanceAlpha) const
{
	// Pivot에서 가까우면 좌우 방향, 끝부분에 가까우면 Pivot Forward 방향으로 힘을 보낸다.
	FVector PivotDirection = Pivot->GetForwardVector();
	PivotDirection.Z = 0.0f;
	PivotDirection.Normalize();

	// 양수 Yaw는 왼쪽 Flipper, 음수 Yaw는 오른쪽 Flipper로 사용한다.
	const FVector TargetDirection = MoveUpRotationOffset.Yaw > 0.0f
		? FVector::LeftVector
		: FVector::RightVector;

	if (PivotDirection.IsNearlyZero())
	{
		return TargetDirection;
	}
	
	// pivot과의 거리가 가까우면 왼쪽 혹은 오른쪽 방향으로 힘을 가함.
	// pivot 과 멀어질 수록 수직으로 힘을 가함.
	const float BlendAlpha = FMath::Sqrt(DistanceAlpha);
	FVector BlendedDirection = FMath::Lerp(TargetDirection, PivotDirection, BlendAlpha);
	BlendedDirection.Z = 0.0f;
	if (!BlendedDirection.Normalize())
	{
		return TargetDirection;
	}

	return BlendedDirection;
}

void AFlipper::SetIsMove(const bool bIsMove)
{
	bIsMoveUp = bIsMove;
}

void AFlipper::ApplyForceToBalls(const float DeltaTime, const float MotionAlpha)
{
	// 현재 Trigger 안에 있는 Ball에게만 이번 프레임의 속도를 전달한다.
	if (DeltaTime <= 0.0f || MotionAlpha <= 0.0f)
	{
		return;
	}

	// 플리퍼가 움직일때만 ball이 충돌하는지 연산함.
	TArray<AActor*> OverlappingActors;
	BallDetectTrigger->GetOverlappingActors(OverlappingActors, APBBallBase::StaticClass());

	for (AActor* OverlappingActor : OverlappingActors)
	{
		APBBallBase* Ball = Cast<APBBallBase>(OverlappingActor);
		IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(Ball);
		if (!IsValid(Ball) || !Movable)
		{
			continue;
		}

		// 위치에 따른 방향과 끝부분 힘 배율을 계산한다.
		const float DistanceAlpha = CalculateDistanceAlpha(Ball->GetActorLocation());
		const FVector ForceDirection = CalculateForceDirection(DistanceAlpha);
		if (ForceDirection.IsNearlyZero())
		{
			continue;
		}

		const float DistanceMultiplier = FMath::Lerp(1.0f, TipPowerMultiplier, DistanceAlpha);
		const float FinalPower = LaunchPower * MotionAlpha * DistanceMultiplier;
		const FVector VelocityToAdd = ForceDirection * FinalPower * DeltaTime;

		Movable->AddVelocity(VelocityToAdd);
	}
}

void AFlipper::DrawForceDirectionDebug() const
{
	// Trigger 구간별 최종 힘 방향을 화살표로 표시한다.
#if ENABLE_DRAW_DEBUG
	if (!GetWorld())
	{
		return;
	}

	constexpr float SampleSpacing = 50.0f;
	constexpr float ArrowLength = 80.0f;
	constexpr float HeightOffset = 10.0f;

	const float HalfLength = BallDetectTrigger->GetUnscaledBoxExtent().Y;
	const int32 SampleCount = FMath::Max(1, FMath::CeilToInt(GetFlipperLength() / SampleSpacing));
	const FTransform TriggerTransform = BallDetectTrigger->GetComponentTransform();

	for (int32 SampleIndex = 0; SampleIndex <= SampleCount; ++SampleIndex)
	{
		const float SampleAlpha = static_cast<float>(SampleIndex) / SampleCount;
		const float LocalY = FMath::Lerp(-HalfLength, HalfLength, SampleAlpha);
		FVector SampleLocation = TriggerTransform.TransformPosition(FVector(0.0f, LocalY, 0.0f));
		SampleLocation.Z += HeightOffset;

		const float DistanceAlpha = CalculateDistanceAlpha(SampleLocation);
		const FVector ForceDirection = CalculateForceDirection(DistanceAlpha);

		DrawDebugDirectionalArrow(
			GetWorld(),
			SampleLocation,
			SampleLocation + ForceDirection * ArrowLength,
			15.0f,
			FColor::Cyan,
			false,
			0.0f,
			0,
			2.0f);
	}
#endif
}
