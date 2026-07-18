// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperEffectBase.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Bumper/Feedback/PBBumperVfxRuntimeComponent.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"

void UPBBumperEffectBase::InitializeEffect(
	APBModularBumperBase* InOwnerBumper,
	const FPBBumperEffectRow& InEffectData)
{
	EffectData = InEffectData;
	Initialize(InOwnerBumper);
}

void UPBBumperEffectBase::Initialize(APBModularBumperBase* InOwnerBumper)
{
	OwnerBumper = InOwnerBumper;
}

void UPBBumperEffectBase::ActivateEffect(APBModularBumperBase* Bumper, APBBallBase* Ball)
{
	ActivateEffectForActor(Bumper, Ball);
}

void UPBBumperEffectBase::ActivateEffectForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	FinishEffect();
}

void UPBBumperEffectBase::FinishEffect()
{
	if (OwnerBumper.IsValid())
	{
		OwnerBumper->FinishActivation();
	}
}

void UPBBumperEffectBase::ShutdownEffect()
{
	OwnerBumper.Reset();
}

FPBBumperEffectRow UPBBumperEffectBase::GetEffectData() const
{
	return EffectData;
}

void UPBBumperEffectBase::PlayDeliveryVfx(AActor* TargetActor, const float Duration) const
{
	if (!OwnerBumper.IsValid() || !IsValid(TargetActor))
	{
		return;
	}

	if (UPBBumperVfxRuntimeComponent* RuntimeVfx =
		UPBBumperVfxRuntimeComponent::FindOrAddToActor(TargetActor))
	{
		RuntimeVfx->PlayAttached(
			MakeVfxChannel(TEXT("Delivery")),
			OwnerBumper->GetDeliveryVfx(),
			FMath::Max(Duration, 0.05f));
	}
}

void UPBBumperEffectBase::PlayImpactVfx(AActor* TargetActor) const
{
	if (OwnerBumper.IsValid() && IsValid(TargetActor))
	{
		UPBBumperVfxRuntimeComponent::PlayImpact(
			OwnerBumper.Get(),
			OwnerBumper->GetImpactVfx(),
			TargetActor);
	}
}

void UPBBumperEffectBase::PlayStatusVfx(AActor* TargetActor, const float Duration) const
{
	if (!OwnerBumper.IsValid() || !IsValid(TargetActor))
	{
		return;
	}

	if (UPBBumperVfxRuntimeComponent* RuntimeVfx =
		UPBBumperVfxRuntimeComponent::FindOrAddToActor(TargetActor))
	{
		RuntimeVfx->PlayAttached(
			MakeVfxChannel(TEXT("Status")),
			OwnerBumper->GetStatusVfx(),
			FMath::Max(Duration, 0.0f));
	}
}

void UPBBumperEffectBase::StopStatusVfx(AActor* TargetActor) const
{
	if (!IsValid(TargetActor))
	{
		return;
	}

	if (UPBBumperVfxRuntimeComponent* RuntimeVfx =
		TargetActor->FindComponentByClass<UPBBumperVfxRuntimeComponent>())
	{
		RuntimeVfx->StopChannel(MakeVfxChannel(TEXT("Status")));
	}
}

void UPBBumperEffectBase::PlayResolvedVfx(
	AActor* TargetActor,
	const float StatusDuration,
	const bool bPlayStatus) const
{
	PlayDeliveryVfx(TargetActor);
	PlayImpactVfx(TargetActor);
	if (bPlayStatus)
	{
		PlayStatusVfx(TargetActor, StatusDuration);
	}
}

FName UPBBumperEffectBase::MakeVfxChannel(const TCHAR* Stage) const
{
	return OwnerBumper.IsValid()
		? FName(*FString::Printf(TEXT("%s_%s"), *OwnerBumper->GetName(), Stage))
		: NAME_None;
}
