#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBRelicSubsystem.generated.h"

class UPBRelicCalculator;

DECLARE_MULTICAST_DELEGATE_OneParam(
	FPBOnRelicAcquired,
	FName);

DECLARE_MULTICAST_DELEGATE_OneParam(
	FPBOnRelicRemoved,
	FName);

DECLARE_MULTICAST_DELEGATE(
	FPBOnRelicsChanged);

UCLASS()
class PINBALLLIKE_API UPBRelicSubsystem
	: public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(
		FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	bool AcquireRelic(FName RelicId);

	bool RemoveRelic(FName RelicId);

	bool HasRelic(FName RelicId) const;

	const TArray<FName>& GetOwnedRelicIds() const;

	UPBRelicCalculator* GetCalculator() const;

	bool GetRandomRelicIds(int32 Count,TArray<FName>& OutRelicIds) const;

	
public:
	FPBOnRelicAcquired OnRelicAcquired;

	FPBOnRelicRemoved OnRelicRemoved;
	
	FPBOnRelicsChanged OnRelicsChanged;

private:
	UPROPERTY()
	TArray<FName> OwnedRelicIds;

	UPROPERTY(Transient)
	TObjectPtr<UPBRelicCalculator> RelicCalculator;
};