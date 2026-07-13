#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBShellCupActor.generated.h"

class UStaticMeshComponent;
class APBShellGameActor;
class UStaticMeshComponent;

UCLASS()
class PINBALLLIKE_API APBShellCupActor : public AActor
{
	GENERATED_BODY()

public:
	APBShellCupActor();

	void SetCupIndex(int32 InIndex);
	int32 GetCupIndex() const { return CupIndex; }

	void SetOwnerGame(APBShellGameActor* InGame);

	void RaiseCup(float Height = 100.f);
	void LowerCup();
	
	void ShowPrize();
	void HidePrize();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleClicked(
		UPrimitiveComponent* TouchedComponent,
		FKey ButtonPressed);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CupMesh;

	UPROPERTY()
	TObjectPtr<APBShellGameActor> OwnerGame;

	UPROPERTY()
	int32 CupIndex = INDEX_NONE;

	FVector InitialLocation;
	
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> PrizeMesh;
	
	
};