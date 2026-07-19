#include "PBOctopusTentacleSlamPattern.h"

#include "Components/SkeletalMeshComponent.h"
#include "PinBallLike/Actor/Boss/Octopus/PBOctopusBoss.h"
#include "PinBallLike/Actor/Boss/Octopus/PBOctopusTentacle.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"
#include "TimerManager.h"

bool UPBOctopusTentacleSlamPattern::CanExecute_Implementation(APBBossBase* Boss) const
{
	const APBOctopusBoss* OctopusBoss = Cast<APBOctopusBoss>(Boss);
	const APBOctopusTentacle* Tentacle = OctopusBoss ? OctopusBoss->GetTentacle(TentacleIndex) : nullptr;

	return Super::CanExecute_Implementation(Boss)
		&& Tentacle
		&& Tentacle->IsTentacleAvailable()
		&& FindPinballActor(Boss)
		&& SlamDuration > 0.0f
		&& DamageWindowStart <= SlamDuration;
}

void UPBOctopusTentacleSlamPattern::ExecutePattern_Implementation(APBBossBase* Boss)
{
	APBOctopusBoss* OctopusBoss = Cast<APBOctopusBoss>(Boss);
	ActiveTentacle = OctopusBoss ? OctopusBoss->GetTentacle(TentacleIndex) : nullptr;
	TargetBall = FindPinballActor(Boss);
	if (!ActiveTentacle || !TargetBall)
	{
		CompleteSlamPattern();
		return;
	}

	FVector SlamDirection = TargetBall->GetActorLocation() - ActiveTentacle->GetActorLocation();
	if (!SlamDirection.Normalize())
	{
		CompleteSlamPattern();
		return;
	}

	USkeletalMeshComponent* TentacleMesh = ActiveTentacle->GetTentacleMesh();
	if (!TentacleMesh)
	{
		CompleteSlamPattern();
		return;
	}

	IsDamageApplied = false;
	IsDamageWindowActive = false;
	TentacleMesh->OnComponentHit.AddUniqueDynamic(this, &UPBOctopusTentacleSlamPattern::HandleTentacleHit);
	ActiveTentacle->OnTentacleDestroyed.AddUniqueDynamic(this, &UPBOctopusTentacleSlamPattern::HandleTentacleDestroyed);
	ActiveTentacle->StartSlam(SlamDirection, SlamDuration);

	FTimerManager& TimerManager = Boss->GetWorldTimerManager();
	if (DamageWindowStart <= 0.0f)
	{
		StartDamageWindow();
	}
	else
	{
		TimerManager.SetTimer(
			DamageWindowStartTimerHandle,
			this,
			&UPBOctopusTentacleSlamPattern::StartDamageWindow,
			DamageWindowStart,
			false);
	}

	TimerManager.SetTimer(
		PatternFinishTimerHandle,
		this,
		&UPBOctopusTentacleSlamPattern::CompleteSlamPattern,
		SlamDuration,
		false);
}

void UPBOctopusTentacleSlamPattern::CancelPatternInternal_Implementation(APBBossBase* Boss)
{
	static_cast<void>(Boss);
	CleanupSlamPattern();
}

void UPBOctopusTentacleSlamPattern::StartDamageWindow()
{
	if (!ActiveTentacle || DamageWindowDuration <= 0.0f || SlamDamage <= 0)
	{
		return;
	}

	IsDamageWindowActive = true;
	UE_LOG(LogTemp, Log, TEXT("[OctopusSlamDamageWindow] Pattern=%s Active=true Duration=%.2f"),
		*GetNameSafe(this),
		DamageWindowDuration);
	if (APBBossBase* Boss = GetOwnerBoss())
	{
		Boss->GetWorldTimerManager().SetTimer(
			DamageWindowFinishTimerHandle,
			this,
			&UPBOctopusTentacleSlamPattern::FinishDamageWindow,
			DamageWindowDuration,
			false);
	}
}

void UPBOctopusTentacleSlamPattern::FinishDamageWindow()
{
	IsDamageWindowActive = false;
	UE_LOG(LogTemp, Log, TEXT("[OctopusSlamDamageWindow] Pattern=%s Active=false"), *GetNameSafe(this));
}

void UPBOctopusTentacleSlamPattern::CompleteSlamPattern()
{
	CleanupSlamPattern();
	FinishPattern();
}

void UPBOctopusTentacleSlamPattern::CleanupSlamPattern()
{
	ClearPatternTimers();
	IsDamageWindowActive = false;

	if (ActiveTentacle)
	{
		if (USkeletalMeshComponent* TentacleMesh = ActiveTentacle->GetTentacleMesh())
		{
			TentacleMesh->OnComponentHit.RemoveDynamic(this, &UPBOctopusTentacleSlamPattern::HandleTentacleHit);
		}

		ActiveTentacle->OnTentacleDestroyed.RemoveDynamic(this, &UPBOctopusTentacleSlamPattern::HandleTentacleDestroyed);
		ActiveTentacle->FinishSlam();
	}

	ActiveTentacle = nullptr;
	TargetBall = nullptr;
}

void UPBOctopusTentacleSlamPattern::ClearPatternTimers()
{
	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		return;
	}

	FTimerManager& TimerManager = Boss->GetWorldTimerManager();
	TimerManager.ClearTimer(DamageWindowStartTimerHandle);
	TimerManager.ClearTimer(DamageWindowFinishTimerHandle);
	TimerManager.ClearTimer(PatternFinishTimerHandle);
}

void UPBOctopusTentacleSlamPattern::HandleTentacleHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	static_cast<void>(HitComponent);
	static_cast<void>(OtherComponent);
	static_cast<void>(NormalImpulse);
	static_cast<void>(Hit);

	UE_LOG(LogTemp, Log, TEXT("[OctopusSlamHit] Pattern=%s OtherActor=%s TargetBall=%s DamageWindowActive=%s DamageApplied=%s"),
		*GetNameSafe(this),
		*GetNameSafe(OtherActor),
		*GetNameSafe(TargetBall),
		IsDamageWindowActive ? TEXT("true") : TEXT("false"),
		IsDamageApplied ? TEXT("true") : TEXT("false"));

	if (!IsDamageWindowActive || IsDamageApplied || OtherActor != TargetBall)
	{
		return;
	}

	IDamageable* Damageable = PBInterfaceUtils::FindInterface<IDamageable>(OtherActor);
	if (!Damageable || Damageable->IsDead())
	{
		return;
	}

	Damageable->TakeDamage(SlamDamage);
	UE_LOG(LogTemp, Log, TEXT("[BossPatternDamage] Pattern=%s Damage=%d Target=%s"),
		*GetNameSafe(this),
		SlamDamage,
		*GetNameSafe(OtherActor));
	IsDamageApplied = true;
}

void UPBOctopusTentacleSlamPattern::HandleTentacleDestroyed()
{
	CompleteSlamPattern();
}
