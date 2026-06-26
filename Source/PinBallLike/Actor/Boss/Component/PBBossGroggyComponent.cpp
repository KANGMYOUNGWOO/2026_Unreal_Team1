#include "PBBossGroggyComponent.h"

#include "GameFramework/Actor.h"

UPBBossGroggyComponent::UPBBossGroggyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBossGroggyComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor = GetOwner();
	MaxGroggyGauge = FMath::Max(MaxGroggyGauge, 1);
	GroggyGauge = FMath::Clamp(GroggyGauge, 0, MaxGroggyGauge);
	OnGroggyGaugeChanged.Broadcast(GroggyGauge, MaxGroggyGauge);

	if (!CanNotifyOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("BossGroggyComponent owner does not implement BossInterface."));
	}
}

void UPBBossGroggyComponent::ApplyGroggyDamage(FName GroggyPointName)
{
	if (IsGroggy)
	{
		return;
	}

	const int32 GroggyAmount = GetGroggyAmount(GroggyPointName);
	const int32 PreviousGroggyGauge = GroggyGauge;

	GroggyGauge = FMath::Clamp(GroggyGauge + GroggyAmount, 0, MaxGroggyGauge);
	OnGroggyGaugeChanged.Broadcast(GroggyGauge, MaxGroggyGauge);

	UE_LOG(LogTemp, Warning, TEXT("Boss Groggy Damaged: %s, Groggy %d -> %d / %d"),
		*GroggyPointName.ToString(),
		PreviousGroggyGauge,
		GroggyGauge,
		MaxGroggyGauge);

	if (GroggyGauge >= MaxGroggyGauge)
	{
		IsGroggy = true;
		UE_LOG(LogTemp, Warning, TEXT("Boss Groggy Triggered."));

		if (CanNotifyOwner())
		{
			IBossInterface::Execute_OnGroggyTriggered(OwnerActor);
		}
	}
}

void UPBBossGroggyComponent::ResetGroggy()
{
	IsGroggy = false;
	GroggyGauge = 0;
	OnGroggyGaugeChanged.Broadcast(GroggyGauge, MaxGroggyGauge);

	UE_LOG(LogTemp, Warning, TEXT("Boss Groggy Reset."));
}

int32 UPBBossGroggyComponent::GetGroggyAmount(FName GroggyPointName) const
{
	if (const FBossGroggyPointData* GroggyPointData = GroggyPointDataMap.Find(GroggyPointName))
	{
		return GroggyPointData->GroggyAmount;
	}

	return DefaultGroggyAmount;
}

bool UPBBossGroggyComponent::CanNotifyOwner() const
{
	return OwnerActor && OwnerActor->GetClass()->ImplementsInterface(UBossInterface::StaticClass());
}
