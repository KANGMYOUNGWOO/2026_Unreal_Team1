// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/StreamableManager.h"
#include "Engine/StaticMesh.h"
#include "AssetStreamingSubsystem.generated.h"

/**
 * 
 */

DECLARE_DELEGATE_OneParam(FOnBallMeshLoaded, const TArray<UStaticMesh*>&);

UCLASS()
class PINBALLLIKE_API UAssetStreamingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public :
	virtual void Deinitialize() override;
	
	void PreLoadBallMesh(const TArray<TSoftObjectPtr<UStaticMesh>>& BallMeshPtr, 
		FOnBallMeshLoaded OnLoaded);
	
	void ReleaseBallMesh();

	private:
	
	void OnBallMeshLoaded(TArray<FSoftObjectPath> LoadedPaths, FOnBallMeshLoaded OnLoaded);
	
	TSharedPtr<FStreamableHandle> BallMeshLoadHandle;
	
	UPROPERTY()
	TMap<FSoftObjectPath, TObjectPtr<UStaticMesh>> LoadedBallMeshes;
	
	
	
};
