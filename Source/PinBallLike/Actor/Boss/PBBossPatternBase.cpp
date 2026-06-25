#include "PBBossPatternBase.h"

#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Component/PBBossPatternComponent.h"
#include "PinBallLike/Actor/Boss/PBBossPatternTelegraph.h"

void UPBBossPatternBase::InitializePattern(UPBBossPatternComponent* NewOwnerPatternComponent)
{
	OwnerPatternComponent = NewOwnerPatternComponent;
}

bool UPBBossPatternBase::CanExecute_Implementation(APBBossBase* Boss) const
{
	return IsEnabled && Boss != nullptr;
}

void UPBBossPatternBase::StartPattern_Implementation(APBBossBase* Boss)
{
	FinishPattern();
}

void UPBBossPatternBase::CancelPattern_Implementation(APBBossBase* Boss)
{
	DestroySpawnedTelegraphs();
}

void UPBBossPatternBase::FinishPattern()
{
	DestroySpawnedTelegraphs();

	if (OwnerPatternComponent)
	{
		OwnerPatternComponent->NotifyPatternFinished(this);
	}
}

UPBBossPatternComponent* UPBBossPatternBase::GetOwnerPatternComponent() const
{
	return OwnerPatternComponent;
}

TArray<APBBossPatternTelegraph*> UPBBossPatternBase::SpawnTelegraph(APBBossBase* Boss)
{
	TArray<APBBossPatternTelegraph*> SpawnedTelegraphList;
	DestroySpawnedTelegraphs();

	if (!Boss)
	{
		return SpawnedTelegraphList;
	}

	UWorld* World = Boss->GetWorld();
	if (!World)
	{
		return SpawnedTelegraphList;
	}

	const FTransform BossTransform = Boss->GetActorTransform();

	for (const FPBBossPatternTelegraphData& TelegraphData : TelegraphDataList)
	{
		if (!TelegraphData.TelegraphClass)
		{
			continue;
		}

		const FVector SpawnLocation = BossTransform.TransformPosition(TelegraphData.Offset);
		const FRotator SpawnRotation = Boss->GetActorRotation() + TelegraphData.Rotation;

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = Boss;
		SpawnParameters.Instigator = Boss;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APBBossPatternTelegraph* Telegraph = World->SpawnActor<APBBossPatternTelegraph>(
			TelegraphData.TelegraphClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParameters);

		if (!Telegraph)
		{
			continue;
		}

		Telegraph->InitTelegraph(TelegraphData.DurationSeconds, TelegraphData.Scale);
		SpawnedTelegraphs.Add(Telegraph);
		SpawnedTelegraphList.Add(Telegraph);
	}

	return SpawnedTelegraphList;
}

float UPBBossPatternBase::GetMaxTelegraphDurationSeconds() const
{
	float MaxDurationSeconds = 0.0f;

	for (const FPBBossPatternTelegraphData& TelegraphData : TelegraphDataList)
	{
		if (!TelegraphData.TelegraphClass)
		{
			continue;
		}

		MaxDurationSeconds = FMath::Max(MaxDurationSeconds, TelegraphData.DurationSeconds);
	}

	return MaxDurationSeconds;
}

void UPBBossPatternBase::DestroySpawnedTelegraphs()
{
	for (APBBossPatternTelegraph* Telegraph : SpawnedTelegraphs)
	{
		if (!IsValid(Telegraph))
		{
			continue;
		}

		Telegraph->DestroyTelegraph();
	}

	SpawnedTelegraphs.Reset();
}
