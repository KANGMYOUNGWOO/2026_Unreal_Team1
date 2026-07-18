#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "PBBumperDragDropOperation.generated.h"

/**
 * 범퍼 카탈로그 카드가 장착 보드로 전달하는 최소 드래그 데이터입니다.
 * 실제 장착 상태는 보관하지 않고 RowName만 전달하여 PlayerDataSubsystem의 기존 저장 흐름을 유지합니다.
 * 장착 가능 카테고리는 드롭 시 현재 데이터 테이블의 Row를 기준으로 다시 검증합니다.
 */
UCLASS()
class PINBALLLIKE_API UPBBumperDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	void InitializeBumperDrag(FName InBumperRowName);

	UFUNCTION(BlueprintPure, Category = "Bumper|DragDrop")
	bool IsValidBumperDrag() const;

	UPROPERTY(BlueprintReadOnly, Category = "Bumper|DragDrop")
	FName BumperRowName = NAME_None;

};
