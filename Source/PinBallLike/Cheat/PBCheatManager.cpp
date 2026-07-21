// Fill out your copyright notice in the Description page of Project Settings.


#include "PBCheatManager.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"
#include "PinBallLike/Actor/StatusEffect/PBBaseStatusEffect.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Component/PBBossGroggyComponent.h"
#include "PinBallLike/Actor/Boss/Component/PBBossStatComponent.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemBoss.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemBossHand.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Subsystem/PBUIManagerSubsystem.h"
#include "PinBallLike/Table/Ball/PBBallAssetIds.h"

namespace
{
const FName LeftGolemHandName = TEXT("Left");
const FName RightGolemHandName = TEXT("Right");
constexpr int32 MaxBumperCheatChargeCount = 100;
constexpr int32 BossCount = 4;

void ShowSimplePopup(UWorld* World)
{
	UGameInstance* GameInstance = IsValid(World) ? World->GetGameInstance() : nullptr;
	UPBUIManagerSubsystem* UIManager = GameInstance
		? GameInstance->GetSubsystem<UPBUIManagerSubsystem>()
		: nullptr;
	if (!UIManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] ShowPopup failed. UIManager is invalid."));
		return;
	}

	if (!UIManager->ShowSimplePopup(FText::FromString(TEXT("Simple popup test."))))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] ShowPopup failed. ShowSimplePopup failed."));
	}
}

FAutoConsoleCommandWithWorld ShowPopupConsoleCommand(
	TEXT("ShowPopup"),
	TEXT("Shows the simple popup test widget in the current game world."),
	FConsoleCommandWithWorldDelegate::CreateStatic(&ShowSimplePopup));

FString GetBumperPositionName(const EPBBumperPositionId PositionId)
{
	const UEnum* PositionEnum = StaticEnum<EPBBumperPositionId>();
	return PositionEnum
		? PositionEnum->GetNameStringByValue(static_cast<int64>(PositionId))
		: TEXT("Unknown");
}

FString GetBumperPositionUsage()
{
	const UEnum* PositionEnum = StaticEnum<EPBBumperPositionId>();
	if (!PositionEnum)
	{
		return TEXT("Bumper position enum is unavailable.");
	}

	TArray<FString> PositionNames;
	for (int32 EnumIndex = 0; EnumIndex < PositionEnum->NumEnums(); ++EnumIndex)
	{
		const FString PositionName = PositionEnum->GetNameStringByIndex(EnumIndex);
		if (PositionName.IsEmpty()
			|| PositionName.Equals(TEXT("MAX"), ESearchCase::IgnoreCase)
			|| PositionName.EndsWith(TEXT("_MAX"), ESearchCase::IgnoreCase))
		{
			continue;
		}

		const int64 EnumValue = PositionEnum->GetValueByIndex(EnumIndex);
		if (EnumValue == static_cast<int64>(EPBBumperPositionId::None))
		{
			continue;
		}

		PositionNames.Add(PositionName);
	}

	return FString::Join(PositionNames, TEXT(", "));
}

bool TryParseBumperPositionName(
	const FName PositionName,
	EPBBumperPositionId& OutPositionId)
{
	OutPositionId = EPBBumperPositionId::None;
	if (PositionName.IsNone())
	{
		return false;
	}

	const UEnum* PositionEnum = StaticEnum<EPBBumperPositionId>();
	if (!PositionEnum)
	{
		return false;
	}

	const int64 EnumValue = PositionEnum->GetValueByNameString(PositionName.ToString());
	if (EnumValue == INDEX_NONE
		|| EnumValue == static_cast<int64>(EPBBumperPositionId::None))
	{
		return false;
	}

	const FString ResolvedName = PositionEnum->GetNameStringByValue(EnumValue);
	if (ResolvedName.IsEmpty()
		|| ResolvedName.Equals(TEXT("MAX"), ESearchCase::IgnoreCase)
		|| ResolvedName.EndsWith(TEXT("_MAX"), ESearchCase::IgnoreCase))
	{
		return false;
	}

	OutPositionId = static_cast<EPBBumperPositionId>(EnumValue);
	return true;
}

bool TryFindUniqueBumperTrigger(
	UWorld* World,
	const EPBBumperPositionId PositionId,
	APBBumperTriggerActorBase*& OutTriggerActor,
	int32& OutMatchCount)
{
	OutTriggerActor = nullptr;
	OutMatchCount = 0;
	if (!IsValid(World) || PositionId == EPBBumperPositionId::None)
	{
		return false;
	}

	for (TActorIterator<APBBumperTriggerActorBase> It(World); It; ++It)
	{
		APBBumperTriggerActorBase* TriggerActor = *It;
		if (!IsValid(TriggerActor) || TriggerActor->GetPositionId() != PositionId)
		{
			continue;
		}

		++OutMatchCount;
		if (!OutTriggerActor)
		{
			OutTriggerActor = TriggerActor;
		}
	}

	return OutMatchCount == 1;
}

bool IsUsableBumperCheatBall(APBBallBase* Ball)
{
	if (!IsValid(Ball) || Ball->IsActorBeingDestroyed())
	{
		return false;
	}

	const UPBBaseResourceComponent* ResourceComponent = Ball->GetResourceComponent();
	return !ResourceComponent || !ResourceComponent->IsDead();
}

APBBallBase* FindBumperCheatInteractionBall(UWorld* World)
{
	if (!IsValid(World))
	{
		return nullptr;
	}

	for (TActorIterator<APBCombatPartyController> It(World); It; ++It)
	{
		APBCombatPartyController* PartyController = *It;
		if (!IsValid(PartyController))
		{
			continue;
		}

		if (APBBallBase* LeaderBall = PartyController->GetLeaderBall();
			IsUsableBumperCheatBall(LeaderBall))
		{
			return LeaderBall;
		}

		for (APBBallBase* PartyBall : PartyController->GetValidPartyBalls())
		{
			if (IsUsableBumperCheatBall(PartyBall))
			{
				return PartyBall;
			}
		}
	}

	for (TActorIterator<APBBallBase> It(World); It; ++It)
	{
		if (APBBallBase* Ball = *It; IsUsableBumperCheatBall(Ball))
		{
			return Ball;
		}
	}

	return nullptr;
}

FName GetCheatSceneMapPath(const int32 SceneIndex)
{
	switch (SceneIndex)
	{
	case 1:
		return FName(TEXT("/Game/Map/Lv_MainMenu"));
	case 2:
		return FName(TEXT("/Game/Map/Lv_Battle"));
	case 3:
		return FName(TEXT("/Game/Map/Lv_Shop"));
	default:
		return NAME_None;
	}
}
}


void UPBCheatManager::AddDefaultBall()
{
	//AddBenchBall(PBBallAssetIds::Ball::Test01);
	AddBenchBall("Ball_Test01");
}

void UPBCheatManager::AddBenchBall(FName BallId)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddBenchBall failed: World is invalid."));
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddBenchBall failed: GameInstance is invalid."));
		return;
	}

	UPBBallDeckSubsystem* DeckSubsystem = GameInstance->GetSubsystem<UPBBallDeckSubsystem>();
	if (!DeckSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddBenchBall failed: BallDeckSubsystem is invalid."));
		return;
	}

	if (!DeckSubsystem->AddNewBallToDeck(BallId))
	{
		UE_LOG(LogTemp, Warning, TEXT("AddBenchBall failed: BallId=%s. Check BallId and empty deck slots."), *BallId.ToString());
		return;
	}

	const FGuid UIRequestId = DeckSubsystem->LoadPlacedBallUIAssetsAsync(FStreamableDelegate());
	const FGuid GameplayRequestId = DeckSubsystem->LoadPlacedBallGameplayAssetsAsync(FStreamableDelegate::CreateUObject(
		this,
		&UPBCheatManager::HandleCheatPlacedBallGameplayAssetsLoaded));

	UE_LOG(LogTemp, Log, TEXT("AddBenchBall succeeded: BallId=%s UIRequestId=%s GameplayRequestId=%s."),
		*BallId.ToString(),
		*UIRequestId.ToString(),
		*GameplayRequestId.ToString());
}

void UPBCheatManager::GoScene(const int32 SceneIndex)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] GoScene failed. World is invalid."));
		return;
	}

	const FName MapPath = GetCheatSceneMapPath(SceneIndex);
	if (MapPath.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] GoScene failed. Unknown scene index=%d. Use 1=MainMenu, 2=Battle, 3=Shop."),
			SceneIndex);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Cheat] Open scene. Index=%d Map=%s"),
		SceneIndex,
		*MapPath.ToString());
	UGameplayStatics::OpenLevel(World, MapPath);
}

void UPBCheatManager::GoMainMenu()
{
	GoScene(1);
}

void UPBCheatManager::GoBattle()
{
	GoScene(2);
}

void UPBCheatManager::SetBossIndex(const int32 BossIndex)
{
	UGameInstance* GameInstance = GetCheatGameInstance();
	UPBPlayerDataSubsystem* PlayerDataSubsystem = GameInstance
		? GameInstance->GetSubsystem<UPBPlayerDataSubsystem>()
		: nullptr;
	if (!PlayerDataSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] SetBossIndex failed. PlayerDataSubsystem is invalid."));
		return;
	}

	if (!PlayerDataSubsystem->SetCurrentBossIndex(BossIndex, BossCount))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] SetBossIndex failed. BossIndex=%d. Use 0~%d."),
			BossIndex,
			BossCount - 1);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Cheat] SetBossIndex succeeded. BossIndex=%d."), BossIndex);
	GoBattle();
}

void UPBCheatManager::GoShop()
{
	GoScene(3);
}

void UPBCheatManager::PrintAsyncLoadState() const
{
	UPBGameDataLoadSubsystem* GameDataLoadSubsystem = GetGameDataLoadSubsystem();
	if (!GameDataLoadSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] PrintAsyncLoadState failed. GameDataLoadSubsystem is invalid."));
		return;
	}

	const TMap<FPrimaryAssetId, TObjectPtr<UObject>>& LoadedPrimaryAssets = GameDataLoadSubsystem->GetLoadedPrimaryAssets();
	UE_LOG(LogTemp, Log, TEXT("[Cheat] AsyncLoadState. PrimaryAssetsReady=%s CachedPrimaryAssets=%d"),
		GameDataLoadSubsystem->IsPrimaryAssetsReady() ? TEXT("true") : TEXT("false"),
		LoadedPrimaryAssets.Num());

	for (const TPair<FPrimaryAssetId, TObjectPtr<UObject>>& LoadedAssetPair : LoadedPrimaryAssets)
	{
		UE_LOG(LogTemp, Log, TEXT("[Cheat] CachedPrimaryAsset. AssetId=%s Asset=%s"),
			*LoadedAssetPair.Key.ToString(),
			*GetNameSafe(LoadedAssetPair.Value.Get()));
	}
}

void UPBCheatManager::DamageBoss(const int32 DamageAmount)
{
	if (DamageAmount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageBoss failed. DamageAmount must be greater than 0."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageBoss failed. World is invalid."));
		return;
	}

	APBBossBase* Boss = Cast<APBBossBase>(UGameplayStatics::GetActorOfClass(World, APBBossBase::StaticClass()));
	if (!Boss)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageBoss failed. Boss is missing."));
		return;
	}

	UPBBossStatComponent* BossStatComponent = Boss->GetBossStatComponent();
	if (!BossStatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageBoss failed. BossStatComponent is invalid."));
		return;
	}

	BossStatComponent->ApplyBossDamage(NAME_None, DamageAmount);
	UE_LOG(LogTemp, Log, TEXT("[Cheat] DamageBoss succeeded. Damage=%d"), DamageAmount);
}

void UPBCheatManager::DamageGolemHand(const FName HandName, const int32 DamageAmount)
{
	if (DamageAmount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageGolemHand failed. DamageAmount must be greater than 0."));
		return;
	}

	EPBGolemBossHandType HandType;
	if (HandName == LeftGolemHandName)
	{
		HandType = EPBGolemBossHandType::Left;
	}
	else if (HandName == RightGolemHandName)
	{
		HandType = EPBGolemBossHandType::Right;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageGolemHand failed. Unknown HandName=%s. Use Left or Right."),
			*HandName.ToString());
		return;
	}

	UWorld* World = GetWorld();
	APBGolemBoss* GolemBoss = World
		? Cast<APBGolemBoss>(UGameplayStatics::GetActorOfClass(World, APBGolemBoss::StaticClass()))
		: nullptr;
	if (!GolemBoss)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageGolemHand failed. GolemBoss is missing."));
		return;
	}

	APBGolemBossHand* GolemHand = GolemBoss->GetGolemHand(HandType);
	if (!GolemHand)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageGolemHand failed. Hand=%s is missing."), *HandName.ToString());
		return;
	}

	GolemHand->ApplyHandDamage(DamageAmount);
	UE_LOG(LogTemp, Log, TEXT("[Cheat] DamageGolemHand succeeded. Hand=%s Damage=%d HP=%d/%d Available=%s"),
		*HandName.ToString(),
		DamageAmount,
		GolemHand->GetCurrentHandHP(),
		GolemHand->GetMaxHandHP(),
		GolemHand->IsHandAvailable() ? TEXT("true") : TEXT("false"));
}

void UPBCheatManager::AddBossGroggy(const int32 GroggyAmount)
{
	if (GroggyAmount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] AddBossGroggy failed. GroggyAmount must be greater than 0."));
		return;
	}

	UWorld* World = GetWorld();
	APBBossBase* Boss = World
		? Cast<APBBossBase>(UGameplayStatics::GetActorOfClass(World, APBBossBase::StaticClass()))
		: nullptr;
	if (!Boss)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] AddBossGroggy failed. Boss is missing."));
		return;
	}

	UPBBossGroggyComponent* BossGroggyComponent = Boss->GetBossGroggyComponent();
	if (!BossGroggyComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] AddBossGroggy failed. BossGroggyComponent is invalid."));
		return;
	}

	BossGroggyComponent->ApplyGroggyDamage(GroggyAmount);
	UE_LOG(LogTemp, Log, TEXT("[Cheat] AddBossGroggy succeeded. Amount=%d Gauge=%d/%d"),
		GroggyAmount,
		BossGroggyComponent->GroggyGauge,
		BossGroggyComponent->MaxGroggyGauge);
}

void UPBCheatManager::DamageBall()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageBall failed. World is invalid."));
		return;
	}

	constexpr int32 DamageAmount = 10;
	int32 DamagedBallCount = 0;
	for (TActorIterator<APBBallBase> It(World); It; ++It)
	{
		APBBallBase* Ball = *It;
		UPBBaseResourceComponent* ResourceComponent =
			IsValid(Ball) ? Ball->GetResourceComponent() : nullptr;
		if (!ResourceComponent || ResourceComponent->IsDead())
		{
			continue;
		}

		ResourceComponent->TakeDamage(DamageAmount);
		++DamagedBallCount;
	}

	UE_LOG(LogTemp, Log,
		TEXT("[Cheat] DamageBall finished. Damage=%d DamagedBalls=%d"),
		DamageAmount,
		DamagedBallCount);
}

void UPBCheatManager::RegenMana()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	constexpr float ManaAmount = 500.0f;
	int32 RestoredBallCount = 0;
	for (TActorIterator<APBBallBase> It(World); It; ++It)
	{
		UPBBaseResourceComponent* ResourceComponent = It->GetResourceComponent();
		if (!ResourceComponent || !ResourceComponent->HasResource(PBResourceNames::Mana))
		{
			continue;
		}

		ResourceComponent->ApplyResourceDelta(PBResourceNames::Mana, ManaAmount);
		++RestoredBallCount;
	}

	UE_LOG(LogTemp, Log,
		TEXT("[Cheat] RestoreBallMana finished. Mana=%.0f RestoredBalls=%d"),
		ManaAmount,
		RestoredBallCount);
}

void UPBCheatManager::ShowBallStatEffect()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	int32 BallCount = 0;
	int32 EffectCount = 0;
	for (TActorIterator<APBBallBase> It(World); It; ++It)
	{
		APBBallBase* Ball = *It;
		UPBStatusEffectComponent* StatusEffectComponent = Ball->GetStatusEffectComponent();
		if (!StatusEffectComponent)
		{
			continue;
		}

		++BallCount;
		TArray<UPBBaseStatusEffect*> ActiveStatusEffects;
		StatusEffectComponent->GetActiveStatusEffects(ActiveStatusEffects);

		UE_LOG(LogTemp, Log,
			TEXT("[Cheat][StatusEffect] Ball=%s BallId=%s InstanceId=%d ActiveEffects=%d"),
			*GetNameSafe(Ball),
			*Ball->GetBallId().ToString(),
			Ball->GetBallInstanceId(),
			ActiveStatusEffects.Num());

		for (const UPBBaseStatusEffect* StatusEffect : ActiveStatusEffects)
		{
			if (!IsValid(StatusEffect))
			{
				continue;
			}

			const FPBStatusEffectRow& EffectRow = StatusEffect->GetStatusEffectRow();
			UE_LOG(LogTemp, Log,
				TEXT("[Cheat][StatusEffect] Effect=%s Stack=%d StackType=%s DurationPolicy=%s Duration=%.2f Interval=%.2f Tags=%s"),
				*StatusEffect->GetStatusEffectId().ToString(),
				StatusEffect->GetStackCount(),
				*UEnum::GetValueAsString(EffectRow.StackType),
				*UEnum::GetValueAsString(EffectRow.DurationPolicy),
				EffectRow.DurationValue,
				EffectRow.Interval,
				*EffectRow.Tags);
			++EffectCount;
		}
	}

	UE_LOG(LogTemp, Log,
		TEXT("[Cheat][StatusEffect] Finished. Balls=%d ActiveEffects=%d"),
		BallCount,
		EffectCount);
}

void UPBCheatManager::ShowPopup()
{
	ShowSimplePopup(GetWorld());
}

void UPBCheatManager::BumperStatus() const
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat][Bumper] Status failed. World is invalid."));
		return;
	}

	TArray<APBBumperTriggerActorBase*> TriggerActors;
	for (TActorIterator<APBBumperTriggerActorBase> It(World); It; ++It)
	{
		if (APBBumperTriggerActorBase* TriggerActor = *It; IsValid(TriggerActor))
		{
			TriggerActors.Add(TriggerActor);
		}
	}

	TriggerActors.Sort(
		[](const APBBumperTriggerActorBase& Left, const APBBumperTriggerActorBase& Right)
		{
			return static_cast<uint8>(Left.GetPositionId())
				< static_cast<uint8>(Right.GetPositionId());
		});

	UE_LOG(LogTemp, Log,
		TEXT("[Cheat][Bumper] Status begin. TriggerCount=%d"),
		TriggerActors.Num());

	for (const APBBumperTriggerActorBase* TriggerActor : TriggerActors)
	{
		const APBModularBumperBase* OwnerBumper = TriggerActor->GetOwnerBumper();
		UE_LOG(LogTemp, Log,
			TEXT("[Cheat][Bumper] Position=%s Count=%d/%d Progress=%s TriggerState=%s BumperState=%s Active=%s Pending=%d Trigger=%s Bumper=%s"),
			*GetBumperPositionName(TriggerActor->GetPositionId()),
			TriggerActor->GetCurrentTriggerCount(),
			TriggerActor->GetRequiredTriggerCount(),
			*UEnum::GetValueAsString(TriggerActor->GetTriggerProgressState()),
			*UEnum::GetValueAsString(TriggerActor->GetTriggerState()),
			IsValid(OwnerBumper)
				? *UEnum::GetValueAsString(OwnerBumper->GetBumperState())
				: TEXT("Invalid"),
			IsValid(OwnerBumper) && OwnerBumper->GetActiveTriggerActor() == TriggerActor
				? TEXT("true")
				: TEXT("false"),
			IsValid(OwnerBumper) ? OwnerBumper->GetPendingActivationCount() : 0,
			*GetNameSafe(TriggerActor),
			*GetNameSafe(OwnerBumper));
	}

	UE_LOG(LogTemp, Log, TEXT("[Cheat][Bumper] Status end."));
}

void UPBCheatManager::BumperCharge(const FName PositionName, const int32 Count)
{
	if (Count <= 0 || Count > MaxBumperCheatChargeCount)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Cheat][Bumper] Charge failed. Count must be in range 1..%d. Count=%d"),
			MaxBumperCheatChargeCount,
			Count);
		return;
	}

	EPBBumperPositionId PositionId;
	if (!TryParseBumperPositionName(PositionName, PositionId))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Cheat][Bumper] Charge failed. Unknown Position=%s. Use one of: %s"),
			*PositionName.ToString(),
			*GetBumperPositionUsage());
		return;
	}

	APBBumperTriggerActorBase* TriggerActor = nullptr;
	int32 MatchCount = 0;
	if (!TryFindUniqueBumperTrigger(GetWorld(), PositionId, TriggerActor, MatchCount))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Cheat][Bumper] Charge failed. Position=%s requires exactly one Trigger but found %d."),
			*GetBumperPositionName(PositionId),
			MatchCount);
		return;
	}

	APBModularBumperBase* OwnerBumper = TriggerActor->GetOwnerBumper();
	if (!IsValid(OwnerBumper))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Cheat][Bumper] Charge failed. Position=%s OwnerBumper is invalid."),
			*GetBumperPositionName(PositionId));
		return;
	}

	APBBallBase* InteractionBall = FindBumperCheatInteractionBall(GetWorld());
	if (!IsValid(InteractionBall))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Cheat][Bumper] Charge failed. No living Ball exists. Prepare or launch the party first."));
		return;
	}

	int32 RequestedHitCount = 0;
	const FHitResult EmptyTriggerHit;
	for (int32 HitIndex = 0; HitIndex < Count; ++HitIndex)
	{
		if (!TriggerActor->CanIncreaseTrigger())
		{
			break;
		}

		OwnerBumper->HandleTriggerActorActivated(
			TriggerActor,
			InteractionBall,
			EmptyTriggerHit);
		++RequestedHitCount;
	}

	UE_LOG(LogTemp, Log,
		TEXT("[Cheat][Bumper] Charge finished. Position=%s Requested=%d AppliedRequests=%d Count=%d/%d Progress=%s BumperState=%s Pending=%d Ball=%s"),
		*GetBumperPositionName(PositionId),
		Count,
		RequestedHitCount,
		TriggerActor->GetCurrentTriggerCount(),
		TriggerActor->GetRequiredTriggerCount(),
		*UEnum::GetValueAsString(TriggerActor->GetTriggerProgressState()),
		*UEnum::GetValueAsString(OwnerBumper->GetBumperState()),
		OwnerBumper->GetPendingActivationCount(),
		*GetNameSafe(InteractionBall));
}

void UPBCheatManager::BumperComplete(const FName PositionName)
{
	EPBBumperPositionId PositionId;
	if (!TryParseBumperPositionName(PositionName, PositionId))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Cheat][Bumper] Complete failed. Unknown Position=%s. Use one of: %s"),
			*PositionName.ToString(),
			*GetBumperPositionUsage());
		return;
	}

	APBBumperTriggerActorBase* TriggerActor = nullptr;
	int32 MatchCount = 0;
	if (!TryFindUniqueBumperTrigger(GetWorld(), PositionId, TriggerActor, MatchCount))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Cheat][Bumper] Complete failed. Position=%s requires exactly one Trigger but found %d."),
			*GetBumperPositionName(PositionId),
			MatchCount);
		return;
	}

	const int32 RemainingCount = FMath::Max(
		TriggerActor->GetRequiredTriggerCount() - TriggerActor->GetCurrentTriggerCount(),
		0);
	if (RemainingCount <= 0 || !TriggerActor->CanIncreaseTrigger())
	{
		UE_LOG(LogTemp, Log,
			TEXT("[Cheat][Bumper] Complete skipped. Position=%s Count=%d/%d Progress=%s TriggerState=%s"),
			*GetBumperPositionName(PositionId),
			TriggerActor->GetCurrentTriggerCount(),
			TriggerActor->GetRequiredTriggerCount(),
			*UEnum::GetValueAsString(TriggerActor->GetTriggerProgressState()),
			*UEnum::GetValueAsString(TriggerActor->GetTriggerState()));
		return;
	}

	BumperCharge(PositionName, RemainingCount);
}

void UPBCheatManager::BumperReset(const FName PositionName)
{
	EPBBumperPositionId PositionId;
	if (!TryParseBumperPositionName(PositionName, PositionId))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Cheat][Bumper] Reset failed. Unknown Position=%s. Use one of: %s"),
			*PositionName.ToString(),
			*GetBumperPositionUsage());
		return;
	}

	APBBumperTriggerActorBase* TriggerActor = nullptr;
	int32 MatchCount = 0;
	if (!TryFindUniqueBumperTrigger(GetWorld(), PositionId, TriggerActor, MatchCount))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Cheat][Bumper] Reset failed. Position=%s requires exactly one Trigger but found %d."),
			*GetBumperPositionName(PositionId),
			MatchCount);
		return;
	}

	APBModularBumperBase* OwnerBumper = TriggerActor->GetOwnerBumper();
	if (!IsValid(OwnerBumper))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Cheat][Bumper] Reset failed. Position=%s OwnerBumper is invalid."),
			*GetBumperPositionName(PositionId));
		return;
	}

	const bool bActiveEffectContinues = OwnerBumper->GetActiveTriggerActor() == TriggerActor;
	OwnerBumper->ResetTriggerCount();

	UE_LOG(LogTemp, Log,
		TEXT("[Cheat][Bumper] Reset finished. Position=%s Count=%d/%d Progress=%s ActiveEffectContinues=%s Pending=%d"),
		*GetBumperPositionName(PositionId),
		TriggerActor->GetCurrentTriggerCount(),
		TriggerActor->GetRequiredTriggerCount(),
		*UEnum::GetValueAsString(TriggerActor->GetTriggerProgressState()),
		bActiveEffectContinues ? TEXT("true") : TEXT("false"),
		OwnerBumper->GetPendingActivationCount());
}

UGameInstance* UPBCheatManager::GetCheatGameInstance() const
{
	UWorld* World = GetWorld();
	return World ? World->GetGameInstance() : nullptr;
}

UPBBallDeckSubsystem* UPBCheatManager::GetBallDeckSubsystem() const
{
	UGameInstance* GameInstance = GetCheatGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;
}

UPBGameDataLoadSubsystem* UPBCheatManager::GetGameDataLoadSubsystem() const
{
	UGameInstance* GameInstance = GetCheatGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>() : nullptr;
}

void UPBCheatManager::RequestPlacedBallAssetReload()
{
	UPBBallDeckSubsystem* DeckSubsystem = GetBallDeckSubsystem();
	if (!DeckSubsystem)
	{
		return;
	}

	// 덱 변경 후 UI/전투용 볼 에셋을 다시 준비한다.
	const FGuid UIRequestId = DeckSubsystem->LoadPlacedBallUIAssetsAsync(FStreamableDelegate());
	const FGuid GameplayRequestId = DeckSubsystem->LoadPlacedBallGameplayAssetsAsync(FStreamableDelegate::CreateUObject(
		this,
		&UPBCheatManager::HandleCheatPlacedBallGameplayAssetsLoaded));

	UE_LOG(LogTemp, Log, TEXT("[Cheat] Reload placed ball assets. UIRequestId=%s GameplayRequestId=%s"),
		*UIRequestId.ToString(),
		*GameplayRequestId.ToString());
}

void UPBCheatManager::HandleCheatPlacedBallGameplayAssetsLoaded()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddBenchBall asset reload finished but World is invalid."));
		return;
	}

	APBCombatPartyController* CombatPartyActor = Cast<APBCombatPartyController>(
		UGameplayStatics::GetActorOfClass(World, APBCombatPartyController::StaticClass()));
	if (!CombatPartyActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddBenchBall asset reload finished but CombatPartyActor is missing."));
		return;
	}

	CombatPartyActor->InitializeFromDeck();
	UE_LOG(LogTemp, Log, TEXT("AddBenchBall refreshed CombatPartyActor after asset reload."));
}
