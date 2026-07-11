#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "PBCollectionDemoActor.generated.h"

class UPBCollectionWidget;
class UPBUserWidget;

/**
 * 도감 전용 테스트 레벨에서만 사용하는 개발 보조 액터입니다.
 * IsEditorOnly()가 true이므로 패키징된 실제 게임에는 포함되지 않습니다.
 */
UCLASS()
class PINBALLLIKE_API APBCollectionDemoActor : public AActor
{
	GENERATED_BODY()

public:
	APBCollectionDemoActor();
	virtual bool IsEditorOnly() const override { return true; }

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
