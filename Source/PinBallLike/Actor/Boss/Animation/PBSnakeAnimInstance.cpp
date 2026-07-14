#include "PBSnakeAnimInstance.h"

#include "PinBallLike/Actor/Boss/SnakeBoss.h"

void UPBSnakeAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CacheOwnerSnakeBoss();
}

void UPBSnakeAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerSnakeBoss)
	{
		CacheOwnerSnakeBoss();
	}

	if (!OwnerSnakeBoss)
	{
		ResetAnimationData();
		return;
	}

	UpdateAnimationDataFromOwner();
}

void UPBSnakeAnimInstance::CacheOwnerSnakeBoss()
{
	OwnerSnakeBoss = Cast<ASnakeBoss>(TryGetPawnOwner());
	if (!OwnerSnakeBoss)
	{
		OwnerSnakeBoss = Cast<ASnakeBoss>(GetOwningActor());
	}
}

void UPBSnakeAnimInstance::UpdateAnimationDataFromOwner()
{
	Speed = OwnerSnakeBoss->GetSnakeAnimationSpeed();
	TurnAmount = OwnerSnakeBoss->GetSnakeAnimationTurnAmount();
	MovePhase = OwnerSnakeBoss->GetSnakeAnimationMovePhase();
	IsMoving = OwnerSnakeBoss->IsSnakeAnimationMoving();
	IsChargePoseActive = OwnerSnakeBoss->IsSnakeChargePoseActive();
	ChargePoseAlpha = OwnerSnakeBoss->GetSnakeChargePoseAlpha();
	IsProjectilePoseActive = OwnerSnakeBoss->IsSnakeProjectilePoseActive();
	ProjectilePoseAlpha = OwnerSnakeBoss->GetSnakeProjectilePoseAlpha();
	SnakeSplinePoints = OwnerSnakeBoss->GetSnakeSplinePoints();
	SnakeChargeHeadSplinePoints = OwnerSnakeBoss->GetSnakeChargeHeadSplinePoints();
}

void UPBSnakeAnimInstance::ResetAnimationData()
{
	Speed = 0.0f;
	TurnAmount = 0.0f;
	MovePhase = 0.0f;
	IsMoving = false;
	IsChargePoseActive = false;
	ChargePoseAlpha = 0.0f;
	IsProjectilePoseActive = false;
	ProjectilePoseAlpha = 0.0f;
	SnakeSplinePoints.Reset();
	SnakeChargeHeadSplinePoints.Reset();
}
