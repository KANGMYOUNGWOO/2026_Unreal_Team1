#pragma once

#include "CoreMinimal.h"

/** 시트에서 사용하는 단일 값 텍스트 템플릿을 안전하게 화면 문구로 변환합니다. */
namespace PBTextFormatUtils
{
	/**
	 * TemplateText에 {0}이 있으면 Value로 치환하고, 일반 문구라면 원문을 그대로 반환합니다.
	 * Bumper/Trigger의 {0}은 발동 횟수, Effect의 {0}은 Power처럼 호출부가 의미에 맞는 값을 전달합니다.
	 */
	inline FText FormatSingleValueTemplate(const FText& TemplateText, const FText& Value)
	{
		return TemplateText.ToString().Contains(TEXT("{0}"))
			? FText::Format(TemplateText, Value)
			: TemplateText;
	}
}
