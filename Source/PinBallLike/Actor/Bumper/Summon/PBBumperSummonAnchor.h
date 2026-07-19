#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Struct/Bumper/PBBumperTriggerSpawnInfo.h"
#include "PBBumperSummonAnchor.generated.h"

class UArrowComponent;
class USceneComponent;

UENUM(BlueprintType)
enum class EPBBumperSummonAnchorType : uint8
{
	None,
	Turret
};

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBumperSummonAnchor : public AActor
{
	GENERATED_BODY()

public:
	APBBumperSummonAnchor();

	UFUNCTION(BlueprintPure, Category = "Bumper|Summon|Anchor")
	EPBBumperSummonAnchorType GetAnchorType() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|Summon|Anchor")
	EPBBumperPositionId GetSourcePositionId() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Summon|Anchor")
	TObjectPtr<USceneComponent> SceneRoot;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Summon|Anchor")
	TObjectPtr<UArrowComponent> DirectionArrow;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Summon|Anchor")
	EPBBumperSummonAnchorType AnchorType = EPBBumperSummonAnchorType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Summon|Anchor")
	EPBBumperPositionId SourcePositionId = EPBBumperPositionId::None;
};
