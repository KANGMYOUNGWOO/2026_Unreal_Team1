#include "PBOctopusTentacleSlamPattern.h"

#include "Components/SkeletalMeshComponent.h"
#include "PinBallLike/Actor/Boss/Octopus/PBOctopusBoss.h"
#include "PinBallLike/Actor/Boss/Octopus/PBOctopusTentacle.h"
#include "PBOctopusTentacleSlamTelegraph.h"
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

void UPBOctopusTentacleSlamPattern::StartPattern_Implementation(APBBossBase* Boss)
{
	APBOctopusBoss* OctopusBoss = Cast<APBOctopusBoss>(Boss);
	ActiveTentacle = OctopusBoss ? OctopusBoss->GetTentacle(TentacleIndex) : nullptr;
	TargetBall = FindPinballActor(Boss);
	if (!Boss || !ActiveTentacle || !TargetBall)
	{
		FinishPattern();
		return;
	}

	SetOwnerBoss(Boss);
	FVector SlamDirection = TargetBall->GetActorLocation() - ActiveTentacle->GetActorLocation();
	if (!SlamDirection.Normalize())
	{
		FinishPattern();
		return;
	}

	const float CurrentTelegraphDuration = SpawnSlamTelegraph(Boss, SlamDirection);
	if (CurrentTelegraphDuration <= 0.0f)
	{
		BeginSlamAfterTelegraph();
		return;
	}

	Boss->GetWorldTimerManager().SetTimer(
		TelegraphFinishTimerHandle,
		this,
		&UPBOctopusTentacleSlamPattern::BeginSlamAfterTelegraph,
		CurrentTelegraphDuration,
		false);
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

void UPBOctopusTentacleSlamPattern::BeginSlamAfterTelegraph()
{
	if (ActiveTelegraph)
	{
		ActiveTelegraph->DestroyTelegraph();
		ActiveTelegraph = nullptr;
	}

	APBBossBase* Boss = GetOwnerBoss();
	if (!Boss)
	{
		FinishPattern();
		return;
	}

	ExecutePattern(Boss);
}

float UPBOctopusTentacleSlamPattern::SpawnSlamTelegraph(
	APBBossBase* Boss,
	const FVector& SlamDirection)
{
	USkeletalMeshComponent* TentacleMesh = ActiveTentacle ? ActiveTentacle->GetTentacleMesh() : nullptr;
	if (!TentacleMesh)
	{
		return 0.0f;
	}

	UWorld* World = Boss ? Boss->GetWorld() : nullptr;
	if (!World || TelegraphDuration <= 0.0f)
	{
		return 0.0f;
	}

	TentacleMesh->UpdateBounds();
	TArray<double> TentacleDimensions = {
		TentacleMesh->Bounds.BoxExtent.X * 2.0f,
		TentacleMesh->Bounds.BoxExtent.Y * 2.0f,
		TentacleMesh->Bounds.BoxExtent.Z * 2.0f
	};
	TentacleDimensions.Sort(TGreater<double>());
	const double TentacleLength = TentacleDimensions[0];
	const double TentacleWidth = TentacleDimensions[1];
	FVector TelegraphDirection(SlamDirection.X, SlamDirection.Y, 0.0);
	if (!TelegraphDirection.Normalize())
	{
		return 0.0f;
	}

	USceneComponent* TelegraphStartComponent = ActiveTentacle->GetTelegraphStartComponent();
	if (!TelegraphStartComponent)
	{
		return 0.0f;
	}

	const FVector SlamOriginLocation = TelegraphStartComponent->GetComponentLocation();
	FVector TelegraphLocation = SlamOriginLocation
		+ TelegraphDirection * (TentacleLength * 0.5f);
	TelegraphLocation.Z = SlamOriginLocation.Z;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Boss;
	SpawnParameters.Instigator = Boss;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	UClass* SpawnClass = TelegraphClass
		? TelegraphClass.Get()
		: APBOctopusTentacleSlamTelegraph::StaticClass();
	ActiveTelegraph = World->SpawnActor<APBOctopusTentacleSlamTelegraph>(
		SpawnClass,
		TelegraphLocation,
		FRotator::ZeroRotator,
		SpawnParameters);
	if (!ActiveTelegraph)
	{
		return 0.0f;
	}

	ActiveTelegraph->InitSlamTelegraph(
		TelegraphStartComponent,
		TargetBall,
		TentacleLength,
		TentacleWidth,
		TelegraphDuration);
	return TelegraphDuration;
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
	if (ActiveTelegraph)
	{
		ActiveTelegraph->DestroyTelegraph();
		ActiveTelegraph = nullptr;
	}

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
	TimerManager.ClearTimer(TelegraphFinishTimerHandle);
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
