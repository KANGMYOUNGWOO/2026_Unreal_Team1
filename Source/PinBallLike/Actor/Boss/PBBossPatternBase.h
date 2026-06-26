#pragma once

#include "CoreMinimal.h"
#include "PBBossPatternTelegraphData.h"
#include "UObject/Object.h"
#include "PBBossPatternBase.generated.h"

class APBBossBase;
class APBBossPatternTelegraph;
class UPBBossPatternComponent;

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API UPBBossPatternBase : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void InitializePattern(UPBBossPatternComponent* NewOwnerPatternComponent);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss|Pattern")
	bool CanExecute(APBBossBase* Boss) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss|Pattern")
	void StartPattern(APBBossBase* Boss);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss|Pattern")
	void CancelPattern(APBBossBase* Boss);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss|Pattern")
	void ExecutePattern(APBBossBase* Boss);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss|Pattern")
	void CancelPatternInternal(APBBossBase* Boss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void FinishPattern();

	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	UPBBossPatternComponent* GetOwnerPatternComponent() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	FName PatternName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern", meta = (ClampMin = "0"))
	float CooldownSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	bool IsEnabled = true;

protected:
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const;
	virtual void StartPattern_Implementation(APBBossBase* Boss);
	virtual void CancelPattern_Implementation(APBBossBase* Boss);
	virtual void ExecutePattern_Implementation(APBBossBase* Boss);
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss);
	virtual void ExecuteNativePattern(APBBossBase* Boss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Telegraph")
	TArray<APBBossPatternTelegraph*> SpawnTelegraph(APBBossBase* Boss);

	void DestroySpawnedTelegraphs();
	APBBossBase* GetOwnerBoss() const;
	void SetOwnerBoss(APBBossBase* Boss);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Pattern|Telegraph")
	TArray<FPBBossPatternTelegraphData> TelegraphDataList;

private:
	void StartExecutePattern();
	void ClearTelegraphTimer();
	float GetMaxTelegraphDurationSeconds() const;

	UPROPERTY(Transient)
	TObjectPtr<UPBBossPatternComponent> OwnerPatternComponent;

	UPROPERTY(Transient)
	TObjectPtr<APBBossBase> OwnerBoss;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APBBossPatternTelegraph>> SpawnedTelegraphs;

	FTimerHandle TelegraphTimerHandle;
};
