#include "Powerplay/Bus/BusPowerplayZone.h"

void ABusPowerplayZone::OnPowerplayTriggered()
{
	if (IsValid(TargetBus))
	{
		TargetBus->TriggerExplosion();
	}
}
