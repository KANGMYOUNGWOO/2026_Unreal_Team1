#include "PBBossPatternBase.h"

#include "PBBossPatternComponent.h"

void UPBBossPatternBase::InitializePattern(UPBBossPatternComponent* NewOwnerPatternComponent)
{
	OwnerPatternComponent = NewOwnerPatternComponent;
}

bool UPBBossPatternBase::CanExecute_Implementation(APBBossBase* Boss) const
{
	return IsEnabled && Boss != nullptr;
}

void UPBBossPatternBase::StartPattern_Implementation(APBBossBase* Boss)
{
	FinishPattern();
}

void UPBBossPatternBase::CancelPattern_Implementation(APBBossBase* Boss)
{
}

void UPBBossPatternBase::FinishPattern()
{
	if (OwnerPatternComponent)
	{
		OwnerPatternComponent->NotifyPatternFinished(this);
	}
}

UPBBossPatternComponent* UPBBossPatternBase::GetOwnerPatternComponent() const
{
	return OwnerPatternComponent;
}
