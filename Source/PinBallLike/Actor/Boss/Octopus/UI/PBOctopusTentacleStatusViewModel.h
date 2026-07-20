#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PBOctopusTentacleStatusViewModel.generated.h"

class APBOctopusBoss;
class APBOctopusTentacle;

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBOctopusTentacleStatusViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Octopus Tentacle ViewModel")
	void SetOctopusBoss(APBOctopusBoss* NewOctopusBoss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Octopus Tentacle ViewModel")
	void ClearOctopusBoss();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Octopus Tentacle ViewModel")
	float Tentacle1HPPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Octopus Tentacle ViewModel")
	float Tentacle2HPPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Octopus Tentacle ViewModel")
	float Tentacle3HPPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Octopus Tentacle ViewModel")
	float Tentacle4HPPercent = 0.0f;

protected:
	virtual void BeginDestroy() override;

private:
	UFUNCTION()
	void HandleTentaclesChanged();

	UFUNCTION()
	void HandleTentacle1HPChanged(int32 CurrentTentacleHP, int32 MaxTentacleHP);

	UFUNCTION()
	void HandleTentacle2HPChanged(int32 CurrentTentacleHP, int32 MaxTentacleHP);

	UFUNCTION()
	void HandleTentacle3HPChanged(int32 CurrentTentacleHP, int32 MaxTentacleHP);

	UFUNCTION()
	void HandleTentacle4HPChanged(int32 CurrentTentacleHP, int32 MaxTentacleHP);

	void BindTentacleEvents();
	void UnbindTentacleEvents();
	void RefreshTentacleStatus();
	static float CalculateHPPercent(int32 CurrentHP, int32 MaxHP);

	UPROPERTY(Transient)
	TObjectPtr<APBOctopusBoss> OctopusBoss;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APBOctopusTentacle>> Tentacles;
};
