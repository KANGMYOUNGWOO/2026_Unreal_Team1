#include "PBGameViewportClient.h"

EMouseCursor::Type UPBGameViewportClient::GetCursor(FViewport* InViewport, const int32 X, const int32 Y)
{
	const EMouseCursor::Type Cursor = Super::GetCursor(InViewport, X, Y);
	return Cursor == EMouseCursor::None ? EMouseCursor::Default : Cursor;
}
