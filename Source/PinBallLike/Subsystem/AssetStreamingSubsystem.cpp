// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetStreamingSubsystem.h"
#include "Engine/AssetManager.h"

void UAssetStreamingSubsystem::Deinitialize()
{
	ReleaseBallMesh();
	Super::Deinitialize();
}

void UAssetStreamingSubsystem::ReleaseBallMesh()
{
	LoadedBallMeshes.Empty();
	
	if (BallMeshLoadHandle.IsValid())
	{
		BallMeshLoadHandle->ReleaseHandle();
		BallMeshLoadHandle.Reset();
	}
}

void UAssetStreamingSubsystem::PreLoadBallMesh(const TArray<TSoftObjectPtr<UStaticMesh>>& BallMeshPtr,
	FOnBallMeshLoaded OnLoaded)
{
	ReleaseBallMesh();
	
	TArray<FSoftObjectPath> Paths;
	Paths.Reserve(BallMeshPtr.Num());
	
	for (const TSoftObjectPtr<UStaticMesh>& MeshPtr : BallMeshPtr)
	{
		if (MeshPtr.IsNull()) {continue;}
		
		const FSoftObjectPath Path = MeshPtr.ToSoftObjectPath();
		
		if (!Path.IsValid()) {continue;}
		
		Paths.Add(Path);
	}
	
	if (Paths.IsEmpty())
	{
		TArray<UStaticMesh*> EmptyMeshes;
		OnLoaded.ExecuteIfBound(EmptyMeshes);
		return;
	}
	
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	BallMeshLoadHandle = StreamableManager.RequestAsyncLoad(Paths,
		FStreamableDelegate::CreateUObject(
			this, 
			&UAssetStreamingSubsystem::OnBallMeshLoaded,
			Paths,
			OnLoaded)
			);
	
	
}


void UAssetStreamingSubsystem::OnBallMeshLoaded(TArray<FSoftObjectPath> LoadedPaths, FOnBallMeshLoaded OnLoaded)
{
	LoadedBallMeshes.Empty();
	
	TArray<UStaticMesh*> ResultMeshes;
	ResultMeshes.Reserve(LoadedPaths.Num());
	
	for (const FSoftObjectPath& Path : LoadedPaths)
	{
		UObject* LoadedObject = Path.ResolveObject();
		UStaticMesh* Mesh = Cast<UStaticMesh>(LoadedObject);
		
		if (Mesh)
		{
			LoadedBallMeshes.Add(Path,Mesh);
		}
		
		ResultMeshes.Add(Mesh);
		
		OnLoaded.ExecuteIfBound(ResultMeshes);
	}
}
