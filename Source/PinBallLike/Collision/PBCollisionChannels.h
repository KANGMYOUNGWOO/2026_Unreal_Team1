#pragma once

#include "Engine/EngineTypes.h"

namespace PBCollisionChannels
{
	// Config/DefaultEngine.ini collision object channel mappings.
	inline constexpr ECollisionChannel Boss = ECC_GameTraceChannel1;
	inline constexpr ECollisionChannel Ball = ECC_GameTraceChannel2;
	inline constexpr ECollisionChannel Skill = ECC_GameTraceChannel3;
}
