#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PBEffectRuntimeInterfaces.generated.h"

class AActor;

UINTERFACE(MinimalAPI)
class UPBCollisionDamageModifier : public UInterface
{
	GENERATED_BODY()
};

class PINBALLLIKE_API IPBCollisionDamageModifier
{
	GENERATED_BODY()

public:
	virtual int32 ModifyCollisionDamage(int32 CurrentDamage) = 0;
};

UINTERFACE(MinimalAPI)
class UPBSkillDamageModifier : public UInterface
{
	GENERATED_BODY()
};

class PINBALLLIKE_API IPBSkillDamageModifier
{
	GENERATED_BODY()

public:
	virtual int32 ModifySkillDamage(int32 CurrentDamage) const = 0;
};

UINTERFACE(MinimalAPI)
class UPBEnemyHitListener : public UInterface
{
	GENERATED_BODY()
};

class PINBALLLIKE_API IPBEnemyHitListener
{
	GENERATED_BODY()

public:
	virtual void HandleEnemyHit(AActor* EnemyActor) = 0;
};

UINTERFACE(MinimalAPI)
class UPBComboChangedListener : public UInterface
{
	GENERATED_BODY()
};

class PINBALLLIKE_API IPBComboChangedListener
{
	GENERATED_BODY()

public:
	virtual void HandleComboChanged(int32 CurrentCombo, int32 MaxCombo) = 0;
};

UINTERFACE(MinimalAPI)
class UPBResourceRevivedListener : public UInterface
{
	GENERATED_BODY()
};

class PINBALLLIKE_API IPBResourceRevivedListener
{
	GENERATED_BODY()

public:
	virtual void HandleResourceRevived(FName ResourceName, float ReviveValue) = 0;
};
