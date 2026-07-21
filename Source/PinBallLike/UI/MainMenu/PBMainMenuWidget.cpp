#include "PBMainMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

namespace PBMainMenu
{
	const FVector2D DesignSize(1920.0f, 1080.0f);
	const FVector2D PetalBaseSize(22.0f, 42.0f);
}

UPBMainMenuWidget::UPBMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PetalRandom(20260721)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> SkyFinder(
		TEXT("/Game/Resources/UI/Main/T_Main_Sky.T_Main_Sky"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> BackgroundFinder(
		TEXT("/Game/Resources/UI/Main/T_Main_BG.T_Main_BG"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> LogoFinder(
		TEXT("/Game/Resources/UI/Main/T_Main_Logo.T_Main_Logo"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> CharacterLeftFinder(
		TEXT("/Game/Resources/UI/Main/T_Main_CH01.T_Main_CH01"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> CharacterRightFinder(
		TEXT("/Game/Resources/UI/Main/T_Main_CH02.T_Main_CH02"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> Boss01Finder(
		TEXT("/Game/Resources/UI/Main/T_Main_Boss01.T_Main_Boss01"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> Boss02Finder(
		TEXT("/Game/Resources/UI/Main/T_Main_Boss02.T_Main_Boss02"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> Boss03Finder(
		TEXT("/Game/Resources/UI/Main/T_Main_Boss03.T_Main_Boss03"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> Boss04Finder(
		TEXT("/Game/Resources/UI/Main/T_Main_Boss04.T_Main_Boss04"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> PetalFinder(
		TEXT("/Game/UnityParticle/CasualRPGVFX/leaf01.leaf01"));

	SkyTexture = SkyFinder.Object;
	BackgroundTexture = BackgroundFinder.Object;
	LogoTexture = LogoFinder.Object;
	CharacterLeftTexture = CharacterLeftFinder.Object;
	CharacterRightTexture = CharacterRightFinder.Object;
	BossTextures = {
		Boss01Finder.Object,
		Boss02Finder.Object,
		Boss03Finder.Object,
		Boss04Finder.Object
	};
	PetalTexture = PetalFinder.Object;

	SerpentBossPlacement.Position = FVector2D(-150.0f, -105.0f);
	SerpentBossPlacement.Size = FVector2D(650.0f, 975.0f);
	SerpentBossPlacement.ZOrder = 10;

	OctopusBossPlacement.Position = FVector2D(1505.0f, 20.0f);
	OctopusBossPlacement.Size = FVector2D(620.0f, 496.0f);
	OctopusBossPlacement.ZOrder = 10;

	GolemBossPlacement.Position = FVector2D(1215.0f, 275.0f);
	GolemBossPlacement.Size = FVector2D(470.0f, 705.0f);
	GolemBossPlacement.ZOrder = 12;

	TurtleBossPlacement.Position = FVector2D(420.0f, 555.0f);
	TurtleBossPlacement.Size = FVector2D(550.0f, 314.0f);
	TurtleBossPlacement.ZOrder = 12;
}

void UPBMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildArtLayer();
}

void UPBMainMenuWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsValid(ArtCanvas))
	{
		return;
	}

	AnimationTime += InDeltaTime;
	const FVector2D TargetMousePosition = GetMouseParallaxTarget();
	SmoothedMousePosition = FMath::Vector2DInterpTo(
		SmoothedMousePosition,
		TargetMousePosition,
		InDeltaTime,
		MouseSmoothingSpeed);

	if (IsValid(BackgroundImage))
	{
		BackgroundImage->SetRenderTranslation(SmoothedMousePosition * BackgroundParallaxDistance);
	}

	for (UImage* BossImage : BossImages)
	{
		if (IsValid(BossImage))
		{
			BossImage->SetRenderTranslation(SmoothedMousePosition * BossParallaxDistance);
		}
	}

	for (UImage* CharacterImage : CharacterImages)
	{
		if (IsValid(CharacterImage))
		{
			CharacterImage->SetRenderTranslation(SmoothedMousePosition * CharacterParallaxDistance);
		}
	}

	if (IsValid(LogoImage))
	{
		const float LogoOffset = FMath::Sin(AnimationTime * UE_TWO_PI * LogoFloatFrequency) * LogoFloatAmplitude;
		LogoImage->SetRenderTranslation(FVector2D(0.0f, LogoOffset));
	}

	UpdatePetals(InDeltaTime);
}

void UPBMainMenuWidget::BuildArtLayer()
{
	if (!IsValid(WidgetTree) || IsValid(ArtCanvas))
	{
		return;
	}

	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!IsValid(RootCanvas))
	{
		return;
	}

	HideLegacySkyImage();

	UScaleBox* ArtScaleBox = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("MainArtScaleBox"));
	UCanvasPanelSlot* ScaleBoxSlot = RootCanvas->AddChildToCanvas(ArtScaleBox);
	ScaleBoxSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	ScaleBoxSlot->SetOffsets(FMargin(0.0f));
	ScaleBoxSlot->SetZOrder(-100);
	ArtScaleBox->SetStretch(EStretch::ScaleToFill);
	ArtScaleBox->SetStretchDirection(EStretchDirection::Both);
	ArtScaleBox->SetVisibility(ESlateVisibility::HitTestInvisible);

	USizeBox* DesignSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("MainArtDesignSize"));
	DesignSizeBox->SetWidthOverride(PBMainMenu::DesignSize.X);
	DesignSizeBox->SetHeightOverride(PBMainMenu::DesignSize.Y);
	ArtScaleBox->AddChild(DesignSizeBox);

	ArtCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MainArtCanvas"));
	DesignSizeBox->AddChild(ArtCanvas);

	AddArtImage(SkyTexture, TEXT("MainSkyImage"), FVector2D::ZeroVector, PBMainMenu::DesignSize, 0);

	if (BossTextures.Num() >= 4)
	{
		BossImages.Add(AddArtImage(
			BossTextures[0],
			TEXT("MainBossSerpentImage"),
			SerpentBossPlacement.Position,
			SerpentBossPlacement.Size,
			SerpentBossPlacement.ZOrder));
		BossImages.Add(AddArtImage(
			BossTextures[3],
			TEXT("MainBossOctopusImage"),
			OctopusBossPlacement.Position,
			OctopusBossPlacement.Size,
			OctopusBossPlacement.ZOrder));
		BossImages.Add(AddArtImage(
			BossTextures[1],
			TEXT("MainBossGolemImage"),
			GolemBossPlacement.Position,
			GolemBossPlacement.Size,
			GolemBossPlacement.ZOrder));
		BossImages.Add(AddArtImage(
			BossTextures[2],
			TEXT("MainBossTurtleImage"),
			TurtleBossPlacement.Position,
			TurtleBossPlacement.Size,
			TurtleBossPlacement.ZOrder));
	}

	BackgroundImage = AddArtImage(
		BackgroundTexture,
		TEXT("MainBackgroundImage"),
		FVector2D(-12.0f, -8.0f),
		FVector2D(1944.0f, 1096.0f),
		20);

	BuildPetals();

	CharacterImages.Add(AddArtImage(
		CharacterLeftTexture,
		TEXT("MainCharacterLeftImage"),
		FVector2D(-40.0f, 300.0f),
		FVector2D(720.0f, 1080.0f),
		40));
	CharacterImages.Add(AddArtImage(
		CharacterRightTexture,
		TEXT("MainCharacterRightImage"),
		FVector2D(1240.0f, 275.0f),
		FVector2D(720.0f, 1080.0f),
		40));

	LogoImage = AddArtImage(
		LogoTexture,
		TEXT("MainLogoImage"),
		FVector2D(440.0f, 20.0f),
		FVector2D(1040.0f, 416.0f),
		50);
}

void UPBMainMenuWidget::HideLegacySkyImage() const
{
	if (!IsValid(SkyTexture))
	{
		return;
	}

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		UImage* Image = Cast<UImage>(Widget);
		if (IsValid(Image) && Image->GetBrush().GetResourceObject() == SkyTexture)
		{
			Image->SetVisibility(ESlateVisibility::Collapsed);
		}
	});
}

UImage* UPBMainMenuWidget::AddArtImage(
	UTexture2D* Texture,
	const FName WidgetName,
	const FVector2D& Position,
	const FVector2D& Size,
	const int32 ZOrder)
{
	if (!IsValid(ArtCanvas) || !IsValid(Texture))
	{
		return nullptr;
	}

	UImage* Image = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), WidgetName);
	Image->SetBrushFromTexture(Texture, true);
	Image->SetVisibility(ESlateVisibility::HitTestInvisible);

	UCanvasPanelSlot* ImageSlot = ArtCanvas->AddChildToCanvas(Image);
	ImageSlot->SetPosition(Position);
	ImageSlot->SetSize(Size);
	ImageSlot->SetZOrder(ZOrder);
	return Image;
}

void UPBMainMenuWidget::BuildPetals()
{
	if (!IsValid(PetalTexture) || PetalCount <= 0)
	{
		return;
	}

	PetalImages.Reserve(PetalCount);
	PetalStates.SetNum(PetalCount);
	PetalRandom.Reset();

	for (int32 PetalIndex = 0; PetalIndex < PetalCount; ++PetalIndex)
	{
		UImage* PetalImage = AddArtImage(
			PetalTexture,
			FName(*FString::Printf(TEXT("MainPetal_%02d"), PetalIndex)),
			FVector2D::ZeroVector,
			PBMainMenu::PetalBaseSize,
			30);
		if (!IsValid(PetalImage))
		{
			continue;
		}

		const float PinkAmount = PetalRandom.FRandRange(0.0f, 1.0f);
		const FLinearColor PetalColor = FLinearColor::LerpUsingHSV(
			FLinearColor(1.0f, 0.82f, 0.88f, 0.42f),
			FLinearColor(1.0f, 0.97f, 0.98f, 0.68f),
			PinkAmount);
		PetalImage->SetColorAndOpacity(PetalColor);
		PetalImages.Add(PetalImage);
		ResetPetal(PetalImages.Num() - 1, true);
	}
}

void UPBMainMenuWidget::ResetPetal(const int32 PetalIndex, const bool bInitialPlacement)
{
	if (!PetalStates.IsValidIndex(PetalIndex) || !PetalImages.IsValidIndex(PetalIndex))
	{
		return;
	}

	FPBMainMenuPetalState& State = PetalStates[PetalIndex];
	State.bRightSide = (PetalIndex % 2) != 0;
	State.Position.X = State.bRightSide
		? PetalRandom.FRandRange(1210.0f, 1990.0f)
		: PetalRandom.FRandRange(-70.0f, 710.0f);
	State.Position.Y = bInitialPlacement
		? PetalRandom.FRandRange(-80.0f, 1060.0f)
		: PetalRandom.FRandRange(-150.0f, -30.0f);
	State.Velocity.X = State.bRightSide
		? PetalRandom.FRandRange(-55.0f, -28.0f)
		: PetalRandom.FRandRange(28.0f, 55.0f);
	State.Velocity.Y = PetalRandom.FRandRange(35.0f, 72.0f);
	State.Phase = PetalRandom.FRandRange(0.0f, UE_TWO_PI);
	State.SwaySpeed = PetalRandom.FRandRange(0.7f, 1.5f);
	State.SwayAmount = PetalRandom.FRandRange(12.0f, 30.0f);
	State.Rotation = PetalRandom.FRandRange(0.0f, 360.0f);
	State.RotationSpeed = PetalRandom.FRandRange(-75.0f, 75.0f);
	State.SizeScale = PetalRandom.FRandRange(0.55f, 1.1f);

	if (UImage* PetalImage = PetalImages[PetalIndex])
	{
		PetalImage->SetRenderTransformAngle(State.Rotation);
		if (UCanvasPanelSlot* PetalSlot = Cast<UCanvasPanelSlot>(PetalImage->Slot))
		{
			PetalSlot->SetPosition(State.Position);
			PetalSlot->SetSize(PBMainMenu::PetalBaseSize * State.SizeScale);
		}
	}
}

void UPBMainMenuWidget::UpdatePetals(const float DeltaTime)
{
	const int32 ActivePetalCount = FMath::Min3(PetalImages.Num(), PetalStates.Num(), PetalCount);
	for (int32 PetalIndex = 0; PetalIndex < ActivePetalCount; ++PetalIndex)
	{
		UImage* PetalImage = PetalImages[PetalIndex];
		if (!IsValid(PetalImage))
		{
			continue;
		}

		FPBMainMenuPetalState& State = PetalStates[PetalIndex];
		State.Position += State.Velocity * DeltaTime;
		State.Position.X += FMath::Sin(AnimationTime * State.SwaySpeed + State.Phase) * State.SwayAmount * DeltaTime;
		State.Rotation = FMath::Fmod(State.Rotation + State.RotationSpeed * DeltaTime, 360.0f);

		const bool bOutsideVerticalBounds = State.Position.Y > PBMainMenu::DesignSize.Y + 100.0f;
		const bool bOutsideHorizontalBounds = State.Position.X < -180.0f || State.Position.X > PBMainMenu::DesignSize.X + 180.0f;
		if (bOutsideVerticalBounds || bOutsideHorizontalBounds)
		{
			ResetPetal(PetalIndex, false);
			continue;
		}

		PetalImage->SetRenderTransformAngle(State.Rotation);
		if (UCanvasPanelSlot* PetalSlot = Cast<UCanvasPanelSlot>(PetalImage->Slot))
		{
			PetalSlot->SetPosition(State.Position);
		}
	}
}

FVector2D UPBMainMenuWidget::GetMouseParallaxTarget() const
{
	const APlayerController* PlayerController = GetOwningPlayer();
	if (!IsValid(PlayerController))
	{
		return FVector2D::ZeroVector;
	}

	float MouseX = 0.0f;
	float MouseY = 0.0f;
	int32 ViewportWidth = 0;
	int32 ViewportHeight = 0;
	PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);
	if (!PlayerController->GetMousePosition(MouseX, MouseY) || ViewportWidth <= 0 || ViewportHeight <= 0)
	{
		return FVector2D::ZeroVector;
	}

	const FVector2D NormalizedMousePosition(
		(MouseX / static_cast<float>(ViewportWidth) - 0.5f) * 2.0f,
		(MouseY / static_cast<float>(ViewportHeight) - 0.5f) * 2.0f);
	return NormalizedMousePosition.ClampAxes(-1.0f, 1.0f);
}
