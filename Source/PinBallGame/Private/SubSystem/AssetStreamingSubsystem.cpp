// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/AssetStreamingSubsystem.h"
#include "Engine/AssetManager.h"

void UAssetStreamingSubsystem::Deinitialize()
{
	ReleaseBallImages();
	Super::Deinitialize();
}

void UAssetStreamingSubsystem::PreLoadBallImage(const TArray<TSoftObjectPtr<UTexture2D>>& BallImages,
	FOnBallImagesLoaded OnLoaded)
{
	ReleaseBallImages();
	
	TArray<FSoftObjectPath> Paths;

	for (const TSoftObjectPtr<UTexture2D>& ImagePtr : BallImages)
	{
		if (ImagePtr.IsNull()) continue;
	
		FSoftObjectPath Path = ImagePtr.ToSoftObjectPath();
	
		if (!Path.IsValid()) continue;
	
		Paths.AddUnique(Path);
	}
	
	if (Paths.Num() == 0)
	{
		OnLoaded.ExecuteIfBound();
		return;
	}

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

	BallImageLoadHandle = StreamableManager.RequestAsyncLoad(Paths,FStreamableDelegate::CreateUObject(
		this,&UAssetStreamingSubsystem::OnBallImagesLoaded,
		Paths,
		OnLoaded)
		);
}

UTexture2D* UAssetStreamingSubsystem::GetLoadedTexture(const TSoftObjectPtr<UTexture2D>& BallImage) const
{
	const FSoftObjectPath Path = BallImage.ToSoftObjectPath();
	if (const TObjectPtr<UTexture2D>* Found = LoadedBallImages.Find(Path))
	{
		return Found->Get();
	}
	return nullptr;
}

void UAssetStreamingSubsystem::ReleaseBallImages()
{
	LoadedBallImages.Empty();

	if (BallImageLoadHandle.IsValid())
	{
		BallImageLoadHandle->ReleaseHandle();
		BallImageLoadHandle.Reset();
	}
}

void UAssetStreamingSubsystem::OnBallImagesLoaded(TArray<FSoftObjectPath> LoadedPaths, FOnBallImagesLoaded OnLoaded)
{
	LoadedBallImages.Empty();

	for(const FSoftObjectPath& Path : LoadedPaths)
	{
		UObject* LoadedObject = Path.ResolveObject();
	
		if (UTexture2D* Texture = Cast<UTexture2D>(LoadedObject))
		{
			LoadedBallImages.Add(Path,Texture);
		}
	}

	OnLoaded.ExecuteIfBound();
}
