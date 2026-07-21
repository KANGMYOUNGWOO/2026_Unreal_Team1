#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/UI/PBUserWidget.h"
#include "PBMainMenuWidget.generated.h"

class UCanvasPanel;
class UImage;
class UTexture2D;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBMainMenuArtPlacement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Layout")
	FVector2D Position = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Layout")
	FVector2D Size = FVector2D(100.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Menu|Layout")
	int32 ZOrder = 0;
};

struct FPBMainMenuPetalState
{
	FVector2D Position = FVector2D::ZeroVector;
	FVector2D Velocity = FVector2D::ZeroVector;
	float Phase = 0.0f;
	float SwaySpeed = 0.0f;
	float SwayAmount = 0.0f;
	float Rotation = 0.0f;
	float RotationSpeed = 0.0f;
	float SizeScale = 1.0f;
	bool bRightSide = false;
};

UCLASS(BlueprintType, Blueprintable)
class PINBALLLIKE_API UPBMainMenuWidget : public UPBUserWidget
{
	GENERATED_BODY()

public:
	UPBMainMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void BuildArtLayer();
	void HideLegacySkyImage() const;
	UImage* AddArtImage(
		UTexture2D* Texture,
		const FName WidgetName,
		const FVector2D& Position,
		const FVector2D& Size,
		int32 ZOrder);
	void BuildPetals();
	void ResetPetal(int32 PetalIndex, bool bInitialPlacement);
	void UpdatePetals(float DeltaTime);
	FVector2D GetMouseParallaxTarget() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Motion", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float BackgroundParallaxDistance = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Motion", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float BossParallaxDistance = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Motion", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float CharacterParallaxDistance = 14.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Motion", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MouseSmoothingSpeed = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Motion", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float LogoFloatAmplitude = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Motion", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float LogoFloatFrequency = 0.18f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Layout|Boss", meta = (AllowPrivateAccess = "true", DisplayName = "Serpent Boss"))
	FPBMainMenuArtPlacement SerpentBossPlacement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Layout|Boss", meta = (AllowPrivateAccess = "true", DisplayName = "Octopus Boss"))
	FPBMainMenuArtPlacement OctopusBossPlacement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Layout|Boss", meta = (AllowPrivateAccess = "true", DisplayName = "Golem Boss"))
	FPBMainMenuArtPlacement GolemBossPlacement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Layout|Boss", meta = (AllowPrivateAccess = "true", DisplayName = "Turtle Boss"))
	FPBMainMenuArtPlacement TurtleBossPlacement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Petals", meta = (AllowPrivateAccess = "true", ClampMin = "0", ClampMax = "40"))
	int32 PetalCount = 18;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Art", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> SkyTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Art", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> BackgroundTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Art", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> LogoTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Art", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> CharacterLeftTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Art", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> CharacterRightTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Art", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UTexture2D>> BossTextures;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Art", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> PetalTexture;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> ArtCanvas;

	UPROPERTY(Transient)
	TObjectPtr<UImage> BackgroundImage;

	UPROPERTY(Transient)
	TObjectPtr<UImage> LogoImage;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> BossImages;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> CharacterImages;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> PetalImages;

	TArray<FPBMainMenuPetalState> PetalStates;
	FRandomStream PetalRandom;
	FVector2D SmoothedMousePosition = FVector2D::ZeroVector;
	float AnimationTime = 0.0f;
};
