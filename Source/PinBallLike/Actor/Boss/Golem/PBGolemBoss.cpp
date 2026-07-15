#include "PBGolemBoss.h"

#include "PBGolemBossHand.h"
#include "PinBallLike/Actor/Boss/Component/PBBossGroggyComponent.h"
#include "PinBallLike/Actor/Boss/Component/PBBossPatternComponent.h"
#include "PinBallLike/Actor/Boss/Golem/Pattern/PBGolemBossPatternBase.h"

APBGolemBoss::APBGolemBoss()
{
	BossMovementType = EPBBossMovementType::Fixed;
	LeftHandClass = APBGolemBossHand::StaticClass();
	RightHandClass = APBGolemBossHand::StaticClass();
}

void APBGolemBoss::BeginPlay()
{
	Super::BeginPlay();

	SpawnGolemHands();
	ResetGolemIdleAnimationSyncTime();
	RequestIdleAnimationSync();
	StartHandsAutonomousMove();
}

void APBGolemBoss::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyGolemHands();

	Super::EndPlay(EndPlayReason);
}

void APBGolemBoss::SpawnGolemHands()
{
	DestroyGolemHands();

	LeftHand = SpawnGolemHand(EPBGolemBossHandType::Left, LeftHandOffset);
	RightHand = SpawnGolemHand(EPBGolemBossHandType::Right, RightHandOffset);
	OnGolemHandsChanged.Broadcast();
}

void APBGolemBoss::MoveHandToOffset(EPBGolemBossHandType HandType, FVector TargetOffset, float Duration)
{
	if (APBGolemBossHand* GolemHand = GetGolemHand(HandType))
	{
		GolemHand->StopAutonomousMove();
		GolemHand->MoveToOffset(TargetOffset, Duration);
	}
}

void APBGolemBoss::LaunchHandFistAtActor(EPBGolemBossHandType HandType, AActor* TargetActor, float Duration)
{
	if (APBGolemBossHand* GolemHand = GetGolemHand(HandType))
	{
		GolemHand->LaunchFistAtActor(TargetActor, Duration);
	}
}

void APBGolemBoss::LaunchHandFistAtLocation(EPBGolemBossHandType HandType, FVector TargetWorldLocation, float Duration)
{
	if (APBGolemBossHand* GolemHand = GetGolemHand(HandType))
	{
		GolemHand->LaunchFistAtLocation(TargetWorldLocation, Duration);
	}
}

void APBGolemBoss::ReturnHandToDefaultOffset(EPBGolemBossHandType HandType, float Duration)
{
	if (APBGolemBossHand* GolemHand = GetGolemHand(HandType))
	{
		GolemHand->ReturnToDefaultOffset(Duration);
	}
}

void APBGolemBoss::StartIdleState_Implementation()
{
	Super::StartIdleState_Implementation();

	ResetGolemIdleAnimationSyncTime();
	RequestIdleAnimationSync();
	StartHandsAutonomousMove();
}

void APBGolemBoss::StartPatternState()
{
	StopHandsAutonomousMove();

	Super::StartPatternState();
}

APBGolemBossHand* APBGolemBoss::GetGolemHand(EPBGolemBossHandType HandType) const
{
	return HandType == EPBGolemBossHandType::Left ? LeftHand : RightHand;
}

void APBGolemBoss::HandleGolemHandDestroyed(APBGolemBossHand* DestroyedHand, int32 GroggyAmount)
{
	if (!IsValid(DestroyedHand) || GetGolemHand(DestroyedHand->GetHandType()) != DestroyedHand)
	{
		return;
	}

	if (BossPatternComponent)
	{
		UPBGolemBossPatternBase* GolemPattern = Cast<UPBGolemBossPatternBase>(BossPatternComponent->GetCurrentPattern());
		if (GolemPattern && GolemPattern->UsesHand(DestroyedHand->GetHandType()))
		{
			BossPatternComponent->CancelCurrentPattern();
		}
	}

	if (BossGroggyComponent && GroggyAmount > 0)
	{
		BossGroggyComponent->ApplyGroggyDamage(GroggyAmount);
	}
}

void APBGolemBoss::ResetGolemIdleAnimationSyncTime()
{
	const UWorld* World = GetWorld();
	GolemIdleAnimationSyncStartTime = World ? World->GetTimeSeconds() : 0.0f;
}

float APBGolemBoss::GetGolemIdleAnimationElapsedTime() const
{
	const UWorld* World = GetWorld();
	return World ? FMath::Max(0.0f, World->GetTimeSeconds() - GolemIdleAnimationSyncStartTime) : 0.0f;
}

float APBGolemBoss::GetSyncedGolemIdleAnimationPosition(float AnimationLength) const
{
	const float SafeAnimationLength = FMath::Max(0.1f, AnimationLength);
	return FMath::Fmod(GetGolemIdleAnimationElapsedTime(), SafeAnimationLength);
}

APBGolemBossHand* APBGolemBoss::SpawnGolemHand(EPBGolemBossHandType HandType, FVector HandOffset)
{
	UWorld* World = GetWorld();
	const TSubclassOf<APBGolemBossHand> GolemHandClass = HandType == EPBGolemBossHandType::Left
		? LeftHandClass
		: RightHandClass;

	if (!World || !GolemHandClass)
	{
		return nullptr;
	}

	const FVector SpawnLocation = GetActorTransform().TransformPosition(HandOffset);
	const FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;

	APBGolemBossHand* GolemHand = World->SpawnActor<APBGolemBossHand>(
		GolemHandClass,
		SpawnTransform,
		SpawnParameters);
	if (!GolemHand)
	{
		return nullptr;
	}

	GolemHand->InitializeGolemHand(this, HandType, HandOffset);
	return GolemHand;
}

void APBGolemBoss::RequestIdleAnimationSync()
{
	BP_OnGolemIdleAnimationSyncRequested(GetSyncedGolemIdleAnimationPosition(GolemIdleAnimationLength));

	if (IsValid(LeftHand))
	{
		LeftHand->RequestIdleAnimationSync();
	}

	if (IsValid(RightHand))
	{
		RightHand->RequestIdleAnimationSync();
	}
}

void APBGolemBoss::StartHandsAutonomousMove()
{
	if (IsValid(LeftHand))
	{
		LeftHand->StartAutonomousMove();
	}

	if (IsValid(RightHand))
	{
		RightHand->StartAutonomousMove();
	}
}

void APBGolemBoss::StopHandsAutonomousMove()
{
	if (IsValid(LeftHand))
	{
		LeftHand->StopAutonomousMove();
	}

	if (IsValid(RightHand))
	{
		RightHand->StopAutonomousMove();
	}
}

void APBGolemBoss::DestroyGolemHands()
{
	StopHandsAutonomousMove();

	if (IsValid(LeftHand))
	{
		LeftHand->Destroy();
	}

	if (IsValid(RightHand))
	{
		RightHand->Destroy();
	}

	LeftHand = nullptr;
	RightHand = nullptr;
}
