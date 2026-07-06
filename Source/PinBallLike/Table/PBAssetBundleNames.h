// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace PBAssetBundleNames
{
	inline const FName Gameplay = TEXT("Gameplay");
	inline const FName UI = TEXT("UI");
}


//meta = (AssetBundles = "Gameplay")
//meta = (AssetBundles = "UI")
//UHT(Unreal Header Tool) 가 컴파일 시점에 읽는 메타데이터라서, 하드코딩으로 박는게 최선임.

