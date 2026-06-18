#pragma once

#include "CoreMinimal.h"
#include "BallDataStruct.generated.h"


USTRUCT(BlueprintType)
struct FBallDataStruct : public FTableRowBase
{
	GENERATED_BODY()
    
public :
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BallId;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NameKey;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BallRank;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DescriptionKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> BallImage;
};

