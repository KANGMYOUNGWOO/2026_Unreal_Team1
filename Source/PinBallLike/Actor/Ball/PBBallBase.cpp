// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallBase.h"
#include  "PinBallLike/Relic/PBRelicCalculator.h"
#include "Component/PBBallHitReactionComponent.h"
#include "Component/PBBallComboComponent.h"
#include "Component/PBBallEffectRuntimeComponent.h"
#include "Component/PBBallPhysicsComponent.h"
#include "Component/PBBallResourceComponent.h"
#include "Component/PBBallSkillComponent.h"
#include "Components/BillboardComponent.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Table/Ball/PBBallAssetIds.h"
#include "PinBallLike/Collision/PBCollisionChannels.h"
#include "Components/SphereComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "TimerManager.h"

APBBallBase::APBBallBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Collision
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(25.0f);
	CollisionSphere->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	CollisionSphere->SetSimulatePhysics(false);
	CollisionSphere->SetEnableGravity(false);
	CollisionSphere->SetGenerateOverlapEvents(true);
	CollisionSphere->SetNotifyRigidBodyCollision(true);

	// Visual
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	BillboardComponent->SetupAttachment(CollisionSphere);
	BillboardComponent->SetUsingAbsoluteScale(true);
	BillboardComponent->SetWorldScale3D(FVector(25.0f));
	
	// Stat
	StatComponent = CreateDefaultSubobject<UPBBaseStatComponent>(TEXT("StatComponent"));
	
	// Resource
	ResourceComponent = CreateDefaultSubobject<UPBBallResourceComponent>(TEXT("ResourceComponent"));

	// StatusEffect
	StatusEffectComponent = CreateDefaultSubobject<UPBStatusEffectComponent>(TEXT("StatusEffectComponent"));

	// Effect
	EffectRuntimeComponent = CreateDefaultSubobject<UPBBallEffectRuntimeComponent>(TEXT("EffectRuntimeComponent"));
	
	// Combo
	ComboComponent = CreateDefaultSubobject<UPBBallComboComponent>(TEXT("ComboComponent"));

	// Skill
	SkillComponent = CreateDefaultSubobject<UPBBallSkillComponent>(TEXT("SkillComponent"));
	
	// Physics
	PhysicsComponent = CreateDefaultSubobject<UPBBallPhysicsComponent>(TEXT("PhysicsComponent"));
	PhysicsComponent->InitializeDependencies(CollisionSphere.Get(), StatComponent.Get());
	
	// Hit Reaction
	HitReactionComponent = CreateDefaultSubobject<UPBBallHitReactionComponent>(TEXT("HitReactionComponent"));
	HitReactionComponent->InitializeDependencies(
		PhysicsComponent.Get(),
		StatComponent.Get(),
		ResourceComponent.Get());
}

void APBBallBase::ApplyStatData(const TArray<FPBStatData>& StatData)
{
	for (const FPBStatData& Stat : StatData)
	{
		if (Stat.StatName.IsNone())
		{
			continue;
		}
		StatComponent->SetStat(Stat.StatName, Stat.Value);
	}
}


void APBBallBase::ApplyResourceData(const TArray<FPBResourceData>& ResourceData)
{
	for (const FPBResourceData& Resource : ResourceData)
	{
		if (Resource.ResourceName.IsNone())
		{
			continue;   
		}
		ResourceComponent->SetResource(Resource.ResourceName, Resource.Current, Resource.Max);
		ResourceComponent->SetResourceRegenPerSecond(Resource.ResourceName, Resource.RegenPerSecond);
	}
}

void APBBallBase::InitializeFromBallInstanceData(const FPBBallInstanceData& InBallInstanceData)
{
	BallInstanceData = InBallInstanceData;
	ApplyStatData(BallInstanceData.BaseStats);
	ApplyResourceData(BallInstanceData.BaseResources);
	if (SkillComponent)
	{
		SkillComponent->InitializeSkill(BallInstanceData);
	}
}

void APBBallBase::SetCombatRole(EPBBallPartyRole NewCombatRole)
{
	CombatRole = NewCombatRole;

	const bool bLeader = CombatRole == EPBBallPartyRole::Leader;
	if (PhysicsComponent)
	{
		if (bLeader)
		{
			PhysicsComponent->ResumeMovement();
		}
		else
		{
			PhysicsComponent->StopMovement();
			PhysicsComponent->PauseMovement();
		}
	}

	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionEnabled(bLeader ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		CollisionSphere->SetGenerateOverlapEvents(bLeader);
		CollisionSphere->SetNotifyRigidBodyCollision(bLeader);
	}
}

void APBBallBase::AddBossCollisionIgnoreRequest(UObject* Requester)
{
	if (!IsValid(Requester))
	{
		return;
	}

	BossCollisionIgnoreRequesters.Add(TWeakObjectPtr<UObject>(Requester));
	RefreshBossCollisionResponse();
}

void APBBallBase::RemoveBossCollisionIgnoreRequest(UObject* Requester)
{
	BossCollisionIgnoreRequesters.Remove(TWeakObjectPtr<UObject>(Requester));
	RefreshBossCollisionResponse();
}

void APBBallBase::RemoveInvalidBossCollisionIgnoreRequests()
{
	for (auto It = BossCollisionIgnoreRequesters.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}
}

void APBBallBase::RefreshBossCollisionResponse()
{
	RemoveInvalidBossCollisionIgnoreRequests();
	if (!CollisionSphere)
	{
		return;
	}

	if (!BossCollisionIgnoreRequesters.IsEmpty())
	{
		GetWorldTimerManager().ClearTimer(BossCollisionRestoreTimerHandle);

		if (!bBossCollisionResponseOverridden)
		{
			BossCollisionResponseBeforeIgnore =
				CollisionSphere->GetCollisionResponseToChannel(PBCollisionChannels::Boss);
			bBossCollisionResponseOverridden = true;
		}

		CollisionSphere->SetCollisionResponseToChannel(PBCollisionChannels::Boss, ECR_Ignore);
		return;
	}

	if (bBossCollisionResponseOverridden)
	{
		if (IsOverlappingBoss())
		{
			if (!GetWorldTimerManager().IsTimerActive(BossCollisionRestoreTimerHandle))
			{
				GetWorldTimerManager().SetTimer(
					BossCollisionRestoreTimerHandle,
					this,
					&APBBallBase::RefreshBossCollisionResponse,
					0.02f,
					true);
			}
			return;
		}

		GetWorldTimerManager().ClearTimer(BossCollisionRestoreTimerHandle);
		CollisionSphere->SetCollisionResponseToChannel(
			PBCollisionChannels::Boss,
			BossCollisionResponseBeforeIgnore);
		bBossCollisionResponseOverridden = false;
	}
}

bool APBBallBase::IsOverlappingBoss() const
{
	const UWorld* World = GetWorld();
	if (!World || !CollisionSphere)
	{
		return false;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(PBCollisionChannels::Boss);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BallBossCollisionRestore), false, this);
	return World->OverlapAnyTestByObjectType(
		CollisionSphere->GetComponentLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(CollisionSphere->GetScaledSphereRadius()),
		QueryParams);
}

bool APBBallBase::TryActivateSkill()
{
	if (IsHidden() || CombatRole == EPBBallPartyRole::None)
	{
		return false;
	}
	return SkillComponent && SkillComponent->TryActivateSkill();
}

void APBBallBase::RefreshRelicStats(const UPBRelicCalculator* RelicCalculator)
{
	if (!IsValid(RelicCalculator) ||
		!IsValid(StatComponent))
	{
		return;
	}

	for (const FPBStatData& BaseStat : BallInstanceData.BaseStats)
	{
		if (BaseStat.StatName.IsNone())
		{
			continue;
		}

		const int32 FinalValue =
			RelicCalculator->CalculateBallStat(
				BaseStat.StatName,
				BaseStat.Value);

		StatComponent->SetStat(
			BaseStat.StatName,
			FinalValue);
	}
	
}

void APBBallBase::BeginPlay()
{
	Super::BeginPlay();
	ApplyBallVisualData();
}

void APBBallBase::ApplyBallVisualData()
{
	UTexture2D* BallSprite = ResolveBallSprite();
	if (!BallSprite)
	{
		return;
	}

	if (!BillboardComponent)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[BallVisual] Billboard component not found. BallId=%s Actor=%s"),
			*BallInstanceData.BallId.ToString(),
			*GetNameSafe(this));
		return;
	}

	BillboardComponent->SetSprite(BallSprite);
}

UTexture2D* APBBallBase::ResolveBallSprite() const
{
	if (BallInstanceData.BallId.IsNone())
	{
		return nullptr;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UPBGameDataLoadSubsystem* GameDataLoadSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>() : nullptr;
	if (!GameDataLoadSubsystem)
	{
		return nullptr;
	}

	const FPrimaryAssetId BallAssetId(PBBallAssetIds::Type::BallData, BallInstanceData.BallId);
	const UPBBallDataAsset* BallDataAsset =
		Cast<UPBBallDataAsset>(GameDataLoadSubsystem->GetLoadedPrimaryAsset(BallAssetId));
	return BallDataAsset ? BallDataAsset->BallSprite.Get() : nullptr;
}
