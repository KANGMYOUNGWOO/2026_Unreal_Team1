#include "PBBumperVfxRuntimeComponent.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

UPBBumperVfxRuntimeComponent::UPBBumperVfxRuntimeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UPBBumperVfxRuntimeComponent* UPBBumperVfxRuntimeComponent::FindOrAddToActor(AActor* TargetActor)
{
	if (!IsValid(TargetActor))
	{
		return nullptr;
	}

	if (UPBBumperVfxRuntimeComponent* Existing =
		TargetActor->FindComponentByClass<UPBBumperVfxRuntimeComponent>())
	{
		return Existing;
	}

	UPBBumperVfxRuntimeComponent* Component = NewObject<UPBBumperVfxRuntimeComponent>(
		TargetActor,
		UPBBumperVfxRuntimeComponent::StaticClass(),
		MakeUniqueObjectName(TargetActor, UPBBumperVfxRuntimeComponent::StaticClass(), TEXT("BumperVfxRuntime")));
	if (!IsValid(Component))
	{
		return nullptr;
	}

	TargetActor->AddInstanceComponent(Component);
	Component->RegisterComponent();
	return Component;
}

UNiagaraComponent* UPBBumperVfxRuntimeComponent::PlayAttached(
	const FName Channel,
	UNiagaraSystem* System,
	const float Duration,
	const FVector& RelativeLocation,
	const FVector& Scale)
{
	AActor* OwnerActor = GetOwner();
	USceneComponent* AttachComponent = IsValid(OwnerActor) ? OwnerActor->GetRootComponent() : nullptr;
	if (Channel.IsNone() || !IsValid(System) || !IsValid(AttachComponent))
	{
		return nullptr;
	}

	StopChannel(Channel);
	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		System,
		AttachComponent,
		NAME_None,
		RelativeLocation,
		FRotator::ZeroRotator,
		Scale,
		EAttachLocation::KeepRelativeOffset,
		false,
		ENCPoolMethod::None,
		true,
		true);
	if (!IsValid(NiagaraComponent))
	{
		return nullptr;
	}

	ActiveComponents.Add(Channel, NiagaraComponent);
	if (Duration > 0.0f)
	{
		FTimerHandle& Timer = ExpirationTimers.FindOrAdd(Channel);
		GetWorld()->GetTimerManager().SetTimer(
			Timer,
			FTimerDelegate::CreateWeakLambda(this, [this, Channel]()
			{
				StopChannel(Channel);
			}),
			FMath::Max(Duration, 0.05f),
			false);
	}
	return NiagaraComponent;
}

void UPBBumperVfxRuntimeComponent::StopChannel(const FName Channel)
{
	if (UWorld* World = GetWorld())
	{
		if (FTimerHandle* Timer = ExpirationTimers.Find(Channel))
		{
			World->GetTimerManager().ClearTimer(*Timer);
		}
	}
	ExpirationTimers.Remove(Channel);

	if (TObjectPtr<UNiagaraComponent>* Component = ActiveComponents.Find(Channel))
	{
		if (IsValid(Component->Get()))
		{
			Component->Get()->DeactivateImmediate();
			Component->Get()->DestroyComponent();
		}
	}
	ActiveComponents.Remove(Channel);
}

void UPBBumperVfxRuntimeComponent::StopAll()
{
	TArray<FName> Channels;
	ActiveComponents.GetKeys(Channels);
	for (const FName Channel : Channels)
	{
		StopChannel(Channel);
	}
}

void UPBBumperVfxRuntimeComponent::PlayImpact(
	const UObject* WorldContext,
	UNiagaraSystem* System,
	const AActor* TargetActor,
	const FVector& WorldOffset,
	const FVector& Scale)
{
	if (!IsValid(WorldContext) || !IsValid(System) || !IsValid(TargetActor))
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		WorldContext,
		System,
		TargetActor->GetActorLocation() + WorldOffset,
		TargetActor->GetActorRotation(),
		Scale,
		true,
		true,
		ENCPoolMethod::AutoRelease,
		true);
}

void UPBBumperVfxRuntimeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAll();
	Super::EndPlay(EndPlayReason);
}
