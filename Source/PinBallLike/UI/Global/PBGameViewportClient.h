#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "PBGameViewportClient.generated.h"

UCLASS()
class PINBALLLIKE_API UPBGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:
	virtual EMouseCursor::Type GetCursor(FViewport* InViewport, int32 X, int32 Y) override;
};
