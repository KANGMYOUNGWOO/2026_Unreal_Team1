#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBShellCupActor.generated.h"

class UStaticMeshComponent;
class UBillboardComponent;
class UTexture2D;
class APBShellGameActor;
class APBShellCupActor;

DECLARE_MULTICAST_DELEGATE_OneParam(
	FPBShellCupPresentationFinished,
	APBShellCupActor*);

UCLASS()
class PINBALLLIKE_API APBShellCupActor : public AActor
{
	GENERATED_BODY()

public:
	APBShellCupActor();

	void SetCupIndex(int32 InIndex);
	int32 GetCupIndex() const { return CupIndex; }

	void SetOwnerGame(APBShellGameActor* InGame);

	UFUNCTION(BlueprintCallable, Category = "Shell Game|Selection")
	void SetSelectionEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Shell Game|Selection")
	bool IsSelectionEnabled() const { return bSelectionEnabled; }

	void RaiseCup(float Height = 100.f);
	void LowerCup();
	void ResetCupPresentation();

	UFUNCTION(BlueprintCallable, Category = "Shell Game|Presentation")
	void NotifyRaiseCupFinished();

	UFUNCTION(BlueprintCallable, Category = "Shell Game|Presentation")
	void NotifyLowerCupFinished();

	FPBShellCupPresentationFinished OnRaiseCupFinished;
	FPBShellCupPresentationFinished OnLowerCupFinished;
	
	void ShowPrize();
	void HidePrize();
	void SetPrizeSprite(UTexture2D* PrizeTexture);
	void SetPrizeScale(float UniformScale);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, Category = "Shell Game|Presentation",
		meta = (DisplayName = "Play Raise Cup"))
	void PlayRaiseCup(float Height);
	virtual void PlayRaiseCup_Implementation(float Height);

	UFUNCTION(BlueprintNativeEvent, Category = "Shell Game|Presentation",
		meta = (DisplayName = "Play Lower Cup"))
	void PlayLowerCup();
	virtual void PlayLowerCup_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Shell Game|Presentation",
		meta = (DisplayName = "Reset Cup Presentation"))
	void PlayResetCupPresentation();
	virtual void PlayResetCupPresentation_Implementation();

private:
	UFUNCTION()
	void HandleClicked(
		UPrimitiveComponent* TouchedComponent,
		FKey ButtonPressed);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shell Game|Presentation",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> CupMesh;

	UPROPERTY()
	TObjectPtr<APBShellGameActor> OwnerGame;

	UPROPERTY()
	int32 CupIndex = INDEX_NONE;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Shell Game|Presentation",
		meta = (AllowPrivateAccess = "true"))
	FVector InitialCupMeshRelativeLocation = FVector::ZeroVector;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shell Game|Prize",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBillboardComponent> PrizeBillboard;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Shell Game|Selection",
		meta = (AllowPrivateAccess = "true"))
	bool bSelectionEnabled = false;

	bool bRaisePresentationPending = false;
	bool bLowerPresentationPending = false;
};
