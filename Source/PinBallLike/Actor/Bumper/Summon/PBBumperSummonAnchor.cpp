#include "PBBumperSummonAnchor.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"

APBBumperSummonAnchor::APBBumperSummonAnchor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

#if WITH_EDITORONLY_DATA
	DirectionArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
	DirectionArrow->SetupAttachment(SceneRoot);
	DirectionArrow->SetArrowColor(FColor(64, 220, 255));
	DirectionArrow->SetHiddenInGame(true);
#endif
}

EPBBumperSummonAnchorType APBBumperSummonAnchor::GetAnchorType() const
{
	return AnchorType;
}

EPBBumperPositionId APBBumperSummonAnchor::GetSourcePositionId() const
{
	return SourcePositionId;
}
