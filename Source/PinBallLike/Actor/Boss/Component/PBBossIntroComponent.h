#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PBBossIntroComponent.generated.h"

struct FPBBattlePhaseChangedMessage;
class UPBBossIntroWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBBossIntroStartedSignature);

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossIntroComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBossIntroComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Intro")
	void CompleteBossIntro();

	UPROPERTY(BlueprintAssignable, Category = "Boss|Intro")
	FPBBossIntroStartedSignature OnBossIntroStarted;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Intro")
	TSoftClassPtr<UPBBossIntroWidget> BossIntroWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Intro", meta = (ClampMin = "0"))
	int32 BossIntroWidgetZOrder = 100;

private:
	void RegisterBattlePhaseListener();
	void UnregisterBattlePhaseListener();
	void HandleBattlePhaseChangedMessage(FGameplayTag Channel, const FPBBattlePhaseChangedMessage& Message);
	void CreateBossIntroWidget();
	void RemoveBossIntroWidget();

	UFUNCTION()
	void HandleBossIntroFinished();

	FGameplayMessageListenerHandle BattlePhaseChangedListenerHandle;
	bool IsBossIntroActive = false;

	UPROPERTY(Transient)
	TObjectPtr<UPBBossIntroWidget> BossIntroWidget;
};
