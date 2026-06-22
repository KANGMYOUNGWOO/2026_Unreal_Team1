// Fill out your copyright notice in the Description page of Project Settings.


#include "TestBoss.h"

#include "Components/SphereComponent.h"
#include "PinBallLike/Actor/Ball/BallBase.h"

ATestBoss::ATestBoss()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(100.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetGenerateOverlapEvents(false);
	CollisionSphere->SetNotifyRigidBodyCollision(true);
	CollisionSphere->OnComponentHit.AddDynamic(this, &ATestBoss::HandleCollisionHit);
}

void ATestBoss::HandleCollisionHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (ABallBase* Ball = Cast<ABallBase>(OtherActor))
	{
		ReceiveHitByBall(Ball, Hit);
	}
}
