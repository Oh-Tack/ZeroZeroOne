#include "Powerplay/Plane/PlanePowerplayZone.h"

void APlanePowerplayZone::OnPowerplayTriggered()
{
	if (IsValid(TargetPlane))
	{
		TargetPlane->TriggerCrash();
	}
}
