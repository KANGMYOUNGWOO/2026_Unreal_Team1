// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture2D.h"
#include "AssetStreamingSubsystem.generated.h"

/**
 * 
 */
DECLARE_DELEGATE(FOnCardImagesLoaded)

UCLASS()
class PINBALLGAME_API UAssetStreamingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	
	public:
	virtual void Deinitialize() override;
	
public:
	void PreLoadBallImage(const TArray<TSoftObjectPtr<UTexture2D>>& BallImages,
		FOnCardImagesLoaded OnLoaded
		);
	
	UTexture2D* GetLoadedTexture(const TSoftObjectPtr<UTexture2D>& BallImage) const;
	
	void ReleaseCardImages();
	
private :
	TSharedPtr<FStreamableHandle> BallImageLoadHandle;
	
	UPROPERTY()
	TMap<FSoftObjectPath, TObjectPtr<UTexture2D>> LoadedBallImages;
	
	
private:
	
	void OnCardImagesLoaded(
		TArray<FSoftObjectPath> LoadedPaths,
		FOnCardImagesLoaded OnLoaded);
};
