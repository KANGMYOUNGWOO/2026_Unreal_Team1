#pragma once

#include "CoreMinimal.h"

class SLoadingScreenLayout;

class FPBLoadingScreen
{
public:
	static TSharedRef<SLoadingScreenLayout> CreateLoadingScreenWidget();
};
