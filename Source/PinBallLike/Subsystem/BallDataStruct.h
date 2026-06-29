#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
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
	int32 BallPrice;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SynergyKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> BallMesh;
	
};
