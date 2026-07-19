// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallHitReactionComponent.h"

#include "PBBallPhysicsComponent.h"
#include "PBBallEffectRuntimeComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Engine/World.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Interface/StatProvider.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PinBallLike/Interface/Damageable.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "PinBallLike/Struct/UI/PBDamageLogMessage.h"


UPBBallHitReactionComponent::UPBBallHitReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBallHitReactionComponent::InitializeDependencies(
	UPBBallPhysicsComponent* InPhysicsComponent,
	IStatProvider* InStatProvider,
	IDamageable* InOwnerDamageable)
{
	PhysicsComponent = InPhysicsComponent;
	StatProvider = InStatProvider;
	OwnerDamageable = InOwnerDamageable;
}

void UPBBallHitReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PhysicsComponent)
	{
		PhysicsComponent->OnBallMovementHit.AddDynamic(this, &UPBBallHitReactionComponent::HandleMovementHit);
	}
}

void UPBBallHitReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PhysicsComponent)
	{
		PhysicsComponent->OnBallMovementHit.RemoveAll(this);
	}
	Super::EndPlay(EndPlayReason);
}

void UPBBallHitReactionComponent::HandleMovementHit(const FHitResult& Hit)
{
	ProcessBallContact(Hit);
}

void UPBBallHitReactionComponent::ProcessBallContact(const FHitResult& Hit)
{
	AActor* Owner = GetOwner();
	AActor* OtherActor = Hit.GetActor();
	if (!Owner || !OtherActor || OtherActor == Owner || WasContactProcessedThisFrame(OtherActor))
	{
		return;
	}
	
	if (!OtherActor->GetClass()->ImplementsInterface(UBossInterface::StaticClass()))
	{
		return;
	}

	const int32 BaseDamage = StatProvider ? StatProvider->GetStat(PBStatNames::Attack) : 0;
	UPBBallEffectRuntimeComponent* EffectRuntimeComponent = Owner->FindComponentByClass<UPBBallEffectRuntimeComponent>();
	const int32 Damage = EffectRuntimeComponent
		? EffectRuntimeComponent->ModifyCollisionDamage(BaseDamage)
		: BaseDamage;
	IBossInterface::Execute_DamageToBoss(OtherActor, Damage);
	
	if (Damage > 0)
	{
		if (UGameplayMessageSubsystem::HasInstance(this))
		{
			FPBDamageLogMessage Message;
			Message.LogType = EPBDamageLogType::Ball;
			Message.DamageAmount = Damage;
			Message.HitLocation = Hit.ImpactPoint;
			UGameplayMessageSubsystem::Get(this).BroadcastMessage(
				GameplayTags::Event_UI_DamageLog_Requested,
				Message);
		}

		ApplyManaGainOnDamage();
	}

	if (EffectRuntimeComponent)
	{
		EffectRuntimeComponent->HandleEnemyHit(OtherActor);
	}

	MarkContactProcessed(OtherActor);

	if (OwnerDamageable && !OwnerDamageable->IsDead())
	{
		OwnerDamageable->TakeDamage(1);
	}
}

void UPBBallHitReactionComponent::ApplyManaGainOnDamage()
{
	if (APBCombatPartyController* PartyController = FindOwningPartyController())
	{
		for (APBBallBase* PartyBall : PartyController->GetValidPartyBalls())
		{
			ApplyManaGainToBall(PartyBall);
		}
		return;
	}

	ApplyManaGainToBall(Cast<APBBallBase>(GetOwner()));
}

void UPBBallHitReactionComponent::ApplyManaGainToBall(APBBallBase* Ball) const
{
	if (!IsValid(Ball))
	{
		return;
	}

	const IStatProvider* BallStatProvider = Cast<IStatProvider>(Ball->FindComponentByInterface(UStatProvider::StaticClass()));
	UPBBaseResourceComponent* BallResourceComponent = Ball->GetResourceComponent();
	if (!BallStatProvider || !BallResourceComponent)
	{
		return;
	}

	const int32 ManaRegen = BallStatProvider->GetStat(PBStatNames::ManaRegen);
	if (ManaRegen <= 0)
	{
		return;
	}

	BallResourceComponent->ApplyResourceDelta(
		PBResourceNames::Mana,
		static_cast<float>(ManaRegen) * GetManaGainMultiplier(Ball));
}

APBCombatPartyController* UPBBallHitReactionComponent::FindOwningPartyController() const
{
	APBBallBase* OwnerBall = Cast<APBBallBase>(GetOwner());
	if (!OwnerBall)
	{
		return nullptr;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (APBCombatPartyController* OwnerPartyController = Cast<APBCombatPartyController>(OwnerBall->GetOwner()))
	{
		return OwnerPartyController->ContainsPartyBall(OwnerBall) ? OwnerPartyController : nullptr;
	}

	for (TActorIterator<APBCombatPartyController> It(World); It; ++It)
	{
		APBCombatPartyController* PartyController = *It;
		if (IsValid(PartyController) && PartyController->ContainsPartyBall(OwnerBall))
		{
			return PartyController;
		}
	}

	return nullptr;
}

float UPBBallHitReactionComponent::GetManaGainMultiplier(const APBBallBase* Ball) const
{
	if (!Ball)
	{
		return 1.0f;
	}

	return Ball->GetCombatRole() == EPBBallPartyRole::Follower ? 0.5f : 1.0f;
}

bool UPBBallHitReactionComponent::WasContactProcessedThisFrame(AActor* OtherActor) const
{
	const UWorld* World = GetWorld();
	// 월드가 유효하고, 마지막 처리 시간과 현재 시간이 거의 같으며, 처리된 액터 목록에 OtherActor가 포함되어 있는지 확인합니다.
	return World
		&& FMath::IsNearlyEqual(ProcessedContactTime, World->GetTimeSeconds())
		&& ProcessedContactActors.Contains(TObjectKey<AActor>(OtherActor));
}

void UPBBallHitReactionComponent::MarkContactProcessed(AActor* OtherActor)
{
	if (const UWorld* World = GetWorld())
	{
		const double CurrentTime = World->GetTimeSeconds();
		// 마지막 처리 시간과 현재 시간이 다르다면, 새로운 프레임으로 간주하고 처리된 액터 목록을 초기화합니다.
		if (!FMath::IsNearlyEqual(ProcessedContactTime, CurrentTime))
		{
			ProcessedContactActors.Reset();
			ProcessedContactTime = CurrentTime;
		}
	}

	ProcessedContactActors.Add(TObjectKey<AActor>(OtherActor));
}
