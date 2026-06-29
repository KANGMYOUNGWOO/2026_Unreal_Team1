#include "PBBossGroggyComponent.h"

#include "GameFramework/Actor.h"
#include "PinBallLike/Utils/PBFixedPoint.h"

UPBBossGroggyComponent::UPBBossGroggyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBossGroggyComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor = GetOwner();
	MaxGroggyGauge = FMath::Max(MaxGroggyGauge, 1);
	MaxGroggyGaugeRaw = FPBFixedPoint::ToRawNonNegative(static_cast<float>(MaxGroggyGauge));
	GroggyGaugeRaw = FPBFixedPoint::ClampRaw(FPBFixedPoint::ToRaw(static_cast<float>(GroggyGauge)), 0, MaxGroggyGaugeRaw);
	RefreshDisplayedGroggyGauge();
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
	const int32 GroggyAmountRaw = FPBFixedPoint::ToRaw(static_cast<float>(GroggyAmount));
	const int32 ClampedGroggyGaugeRaw = static_cast<int32>(FMath::Min<int64>(
		static_cast<int64>(GroggyGaugeRaw) + GroggyAmountRaw,
		TNumericLimits<int32>::Max()));

	GroggyGaugeRaw = FPBFixedPoint::ClampRaw(ClampedGroggyGaugeRaw, 0, MaxGroggyGaugeRaw);
	RefreshDisplayedGroggyGauge();
	OnGroggyGaugeChanged.Broadcast(GroggyGauge, MaxGroggyGauge);

	UE_LOG(LogTemp, Warning, TEXT("Boss Groggy Damaged: %s, Groggy %d -> %d / %d"),
		*GroggyPointName.ToString(),
		PreviousGroggyGauge,
		GroggyGauge,
		MaxGroggyGauge);

	if (GroggyGaugeRaw >= MaxGroggyGaugeRaw)
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
	GroggyGaugeRaw = 0;
	RefreshDisplayedGroggyGauge();
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

int32 UPBBossGroggyComponent::GetDisplayedGroggyGauge() const
{
	return FMath::FloorToInt(FPBFixedPoint::ToFloat(GroggyGaugeRaw));
}

void UPBBossGroggyComponent::RefreshDisplayedGroggyGauge()
{
	MaxGroggyGauge = FMath::Max(1, FMath::FloorToInt(FPBFixedPoint::ToFloat(MaxGroggyGaugeRaw)));
	GroggyGauge = FMath::Clamp(GetDisplayedGroggyGauge(), 0, MaxGroggyGauge);
}

bool UPBBossGroggyComponent::CanNotifyOwner() const
{
	return OwnerActor && OwnerActor->GetClass()->ImplementsInterface(UBossInterface::StaticClass());
}
