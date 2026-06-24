#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"

namespace PBInterfaceUtils
{
	template <typename InterfaceType, typename UInterfaceType>
	InterfaceType* FindInterfaceWithUType(AActor* Actor)
	{
		if (!IsValid(Actor))
		{
			return nullptr;
		}

		TArray<UActorComponent*> Components;
		Actor->GetComponents(Components);

		for (UActorComponent* Component : Components)
		{
			if (!IsValid(Component) || !Component->GetClass()->ImplementsInterface(UInterfaceType::StaticClass()))
			{
				continue;
			}

			if (InterfaceType* Interface = Cast<InterfaceType>(Component))
			{
				return Interface;
			}
		}

		return nullptr;
	}

	template <typename InterfaceType, typename UInterfaceType>
	const InterfaceType* FindInterfaceWithUType(const AActor* Actor)
	{
		return FindInterfaceWithUType<InterfaceType, UInterfaceType>(const_cast<AActor*>(Actor));
	}

	template <typename InterfaceType>
	InterfaceType* FindInterface(AActor* Actor)
	{
		if (!IsValid(Actor))
		{
			return nullptr;
		}

		using UInterfaceType = typename InterfaceType::UClassType;

		TArray<UActorComponent*> Components;
		Actor->GetComponents(Components);

		for (UActorComponent* Component : Components)
		{
			if (IsValid(Component) && Component->GetClass()->ImplementsInterface(UInterfaceType::StaticClass()))
			{
				if (InterfaceType* Interface = Cast<InterfaceType>(Component))
				{
					return Interface;
				}
			}
		}

		return nullptr;
	}
	
	template <typename InterfaceType>
	const InterfaceType* FindInterface(const AActor* Actor)
	{
		return FindInterface<InterfaceType>(const_cast<AActor*>(Actor));
	}
}