// Fill out your copyright notice in the Description page of Project Settings.


#include "PBChoiceBallActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
APBChoiceBallActor::APBChoiceBallActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
    BallMesh->SetupAttachment(Root);

    BallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(Root);

    SpringArm->TargetArmLength = 500.f;
    SpringArm->bUsePawnControlRotation = false;
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 6.f;
    SpringArm->bEnableCameraRotationLag = true;
    SpringArm->CameraRotationLagSpeed = 8.f;

    SpringArm->SetRelativeRotation(FRotator(-20.f, 0.f, 0.f));

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(SpringArm);
}

void APBChoiceBallActor::ApplyRouteTransform(
    const FVector& NewLocation,
    const FRotator& NewRotation,
    float DeltaTime)
{
    const FVector OldLocation = GetActorLocation();

    SetActorLocation(NewLocation);
    SetActorRotation(NewRotation);

    if (bRotateVisual)
    {
        RotateBallVisual(OldLocation, NewLocation, DeltaTime);
    }
}

void APBChoiceBallActor::RotateBallVisual(
    const FVector& OldLocation,
    const FVector& NewLocation,
    float DeltaTime)
{
    if (!BallMesh || DeltaTime <= 0.f || BallRadius <= 0.f)
    {
        return;
    }

    const FVector MoveDelta = NewLocation - OldLocation;
    const float MoveDistance = MoveDelta.Size();

    if (MoveDistance <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const FVector MoveDirection = MoveDelta.GetSafeNormal();

    // 공의 회전축: 이동 방향과 월드 Up의 외적
    const FVector RotationAxis = FVector::CrossProduct(FVector::UpVector, MoveDirection).GetSafeNormal();

    if (RotationAxis.IsNearlyZero())
    {
        return;
    }

    // 이동거리 = 반지름 * 회전각(rad)
    const float AngleRadians =
     (MoveDistance / BallRadius) * RotationMultiplier;
    const float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

    const FQuat DeltaRotation(RotationAxis, FMath::DegreesToRadians(AngleDegrees));

    BallMesh->AddLocalRotation(DeltaRotation);
}



