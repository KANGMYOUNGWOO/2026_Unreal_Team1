#pragma once

#include "CoreMinimal.h"

namespace PBTextFormatUtils
{
	inline FText FormatSingleValueTemplate(const FText& TemplateText, const FText& Value)
	{
		return TemplateText.ToString().Contains(TEXT("{0}"))
			? FText::Format(TemplateText, Value)
			: TemplateText;
	}
}
