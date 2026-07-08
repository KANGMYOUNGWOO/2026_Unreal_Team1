#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "PBCollectionDemoActor.generated.h"

class UPBCollectionWidget;
class UPBUserWidget;

/**
 * 도감 프로토타입을 테스트 레벨에서 열기 위한 임시 진입점입니다.
 * 레벨에 배치한 뒤 BeginPlay 자동 열기 또는 ToggleKey 입력으로 UIManager Push / Pop 흐름을 검증합니다.
 */
UCLASS()
class PINBALLLIKE_API APBCollectionDemoActor : public AActor
{
	GENERATED_BODY()

public:
	APBCollectionDemoActor();

	UFUNCTION(BlueprintCallable, Category = "Collection|Demo")
	void OpenCollection();

	UFUNCTION(BlueprintCallable, Category = "Collection|Demo")
	void CloseCollection();

	UFUNCTION(BlueprintCallable, Category = "Collection|Demo")
	void ToggleCollection();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditAnywhere, Category = "Collection|Demo")
	TSubclassOf<UPBCollectionWidget> CollectionWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Collection|Demo")
	bool bOpenOnBeginPlay = false;

	UPROPERTY(EditAnywhere, Category = "Collection|Demo")
	bool bBindToggleKey = true;

	UPROPERTY(EditAnywhere, Category = "Collection|Demo")
	FKey ToggleKey = EKeys::C;

	UPROPERTY(EditAnywhere, Category = "Collection|Demo")
	int32 ViewportZOrder = 50;

	UPROPERTY(Transient)
	TObjectPtr<UPBUserWidget> CollectionWidget;
};
