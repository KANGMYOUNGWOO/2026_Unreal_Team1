#include "GameplayTags.h"

namespace GameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Event_UI_Drag_Started,  "Event.UI.Drag.Started");
	UE_DEFINE_GAMEPLAY_TAG(Event_UI_Drag_Moved,    "Event.UI.Drag.Moved");
	UE_DEFINE_GAMEPLAY_TAG(Event_UI_Drag_Dropped,  "Event.UI.Drag.Dropped");
	UE_DEFINE_GAMEPLAY_TAG(Event_UI_Drag_Cancelled,"Event.UI.Drag.Cancelled");

	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_Phase_Prepare_Completed, "Event.Battle.Phase.Prepare.Completed");
}
