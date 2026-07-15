#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Table/Ball/Struct/PBBallSkillTableRow.h"
#include "PBBallSkillActorBase.generated.h"

class APBBallBase;
class UPBTimedAreaDamageComponent;

UENUM(BlueprintType)
enum class EPBBallSkillActorState : uint8
{
	None,
	Preparing,
	Active,
	Finishing,
	Stopping,
	Completed
};

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API APBBallSkillActorBase : public AActor
{
	GENERATED_BODY()

public:
	virtual void InitializeSkill(
		APBBallBase* InOwnerBall,
		const FPBBallSkillTableRow& InSkillData);

	UFUNCTION(BlueprintCallable, Category = "Ball|Skill|Effect")
	void ActivateSkill();

	UFUNCTION(BlueprintCallable, Category = "Ball|Skill|Effect")
	void FinishSkill();

	UFUNCTION(BlueprintCallable, Category = "Ball|Skill|Effect")
	void StopSkill();

	UFUNCTION(BlueprintCallable, Category = "Ball|Skill|Effect")
	void CompleteSkill();

	UFUNCTION(BlueprintPure, Category = "Ball|Skill|Effect")
	EPBBallSkillActorState GetSkillState() const { return State; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintNativeEvent, Category = "Ball|Skill|Effect")
	void PrepareSkill();
	virtual void PrepareSkill_Implementation();

	virtual void EnterPreparingState();
	virtual void EnterActiveState();
	virtual void EnterFinishingState();
	virtual void EnterStoppingState();
	virtual void EnterCompletedState();

	virtual AActor* FindTarget() const;
	virtual bool IsTargetValid(const AActor* Target) const;
	const FPBBallSkillTableRow& GetSkillData() const { return SkillData; }
	int32 GetSkillDamageAmount() const { return SkillDamageAmount; }

	void BindDamageEvents(UPBTimedAreaDamageComponent* DamageComponent);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball|Skill|Effect")
	TObjectPtr<APBBallBase> OwnerBall;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ball|Skill|Effect")
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ball|Skill|Effect")
	void OnHit(AActor* Target, int32 AppliedDamage, FVector HitLocation);

	UFUNCTION(BlueprintNativeEvent, Category = "Ball|Skill|Effect")
	void OnFinished();
	virtual void OnFinished_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Ball|Skill|Effect")
	void OnStopped();
	virtual void OnStopped_Implementation();

	UFUNCTION()
	void HandleDamageApplied(AActor* Target, int32 AppliedDamage, FVector HitLocation);

	UFUNCTION()
	void HandleAreaDamageFinished();

	UFUNCTION()
	void HandleOwnerBallDestroyed(AActor* DestroyedActor);

private:
	bool ChangeState(EPBBallSkillActorState NewState);
	void UnbindDamageEvents();

	UPROPERTY(Transient)
	TObjectPtr<UPBTimedAreaDamageComponent> BoundDamageComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball|Skill|Effect", meta = (AllowPrivateAccess = "true"))
	EPBBallSkillActorState State = EPBBallSkillActorState::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball|Skill|Data", meta = (AllowPrivateAccess = "true"))
	FPBBallSkillTableRow SkillData;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball|Skill|Data", meta = (AllowPrivateAccess = "true"))
	int32 SkillDamageAmount = 0;
};
