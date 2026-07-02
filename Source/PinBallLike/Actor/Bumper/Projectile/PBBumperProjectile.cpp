// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

APBBumperProjectile::APBBumperProjectile()
{
	if (IsValid(ProjectileMovementComponent))
	{
		ProjectileMovementComponent->InitialSpeed = ProjectileSpeed;
		ProjectileMovementComponent->MaxSpeed = ProjectileSpeed;
	}

	if (IsValid(CollisionSphere))
	{
		CollisionSphere->InitSphereRadius(12.0f);
	}
}
