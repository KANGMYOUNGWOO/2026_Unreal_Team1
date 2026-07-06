#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

namespace PBSubsystemUtils
{
	template <typename SubsystemType>
	SubsystemType* GetGameInstanceSubsystem(const UObject* WorldContextObject)
	{
		if (!IsValid(WorldContextObject))
		{
			return nullptr;
		}

		const UWorld* World = WorldContextObject->GetWorld();
		UGameInstance* GameInstance = IsValid(World) ? World->GetGameInstance() : nullptr;
		return IsValid(GameInstance) ? GameInstance->GetSubsystem<SubsystemType>() : nullptr;
	}
}
